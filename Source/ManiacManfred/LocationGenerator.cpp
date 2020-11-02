// Fill out your copyright notice in the Description page of Project Settings.

#include "LocationGenerator.h"
#include "Engine/World.h"
#include "ArticyReference.h"
#include "Paper2DClasses.h"
#include "UObject/UObjectGlobals.h"

/* This class will take care of populating a Unreal level with elements from an articy location.
*  The goal of this class is to very quickly and easily fill your scene from an articy location and it should give you an idea how you can use the plugin
*  for editor scripts.
* 
*  Note: This is a quick and dirty LocationGenerator and is used specifically for the Maniac Manfred Adventure Demo project, while it might work for your projects, you might have to 
*  modify a lot of it to make it work for you.
*/


void GetAllChildrenRecursive(AActor* Actor, TArray<AActor*>* Children)
{
	auto children = Actor->Children;
	for (int i = 0; i < children.Num(); ++i)
	{
		auto child = children[i];
		Children->Add(child);
		GetAllChildrenRecursive(child, Children);
	}
}

/* Determines the name for an actor depending on the articy object it should represent */
FName GetNameForActor(const UArticyObject* ArticyObject)
{
	// display name > technical name 
	FName finalName = ArticyObject->GetTechnicalName();
	auto displayName = Cast<IArticyObjectWithDisplayName>(ArticyObject);
	if (displayName)
		finalName = FName(*displayName->GetDisplayName().ToString());

	// if the object is an target (e.g. an object link in an articy location),
	// we use the name of the target
	auto target = Cast<IArticyObjectWithTarget>(ArticyObject);
	if (target)
	{
		auto targetObj = Cast<UArticyObject>(target->GetTarget());
		if (targetObj)
			return GetNameForActor(targetObj);
	}

	return finalName;
}

/* Starts the creation process, triggered a Blueprint Node.
*  Deletes previously generated objects, calculates the bounds of the level and starts the recursive object creation of the level.
*/
void ULocationGenerator::GenerateLocation(UManiacManfredLocation* Location, float PixelsToUnits, const TMap<TSubclassOf<class UArticyBaseObject>, TSubclassOf<class UActorComponent>> ObjectComponentMap, AActor* WorldContext, /*OUT*/ UManiacManfredLocationImage*& BackgroundLayer)
{
#if WITH_EDITOR

	// Clear current generated location
	TArray<AActor*> actorChildren;
	GetAllChildrenRecursive(WorldContext, &actorChildren);
	for (auto child : actorChildren)
	{
		if (auto paperSpriteActor = Cast<APaperSpriteActor>(child))
		{
			if (auto sprite = paperSpriteActor->GetRenderComponent()->GetSprite())
			{
				sprite->ConditionalBeginDestroy();
				sprite = NULL;
			}
		}
		child->Destroy();
	}

	// here we calculate the bounds of the 2D elements we are going to create
	FRect overallBounds = FRect();
	auto db = UArticyDatabase::Get(WorldContext);
	TArray<TWeakObjectPtr<UArticyObject>> children = Location->GetChildren();
	for (auto child : children)
	{
		// the only elements that could actually change the bounds are objects with vertices(Zones, images etc)
		auto objWithVertices = Cast<IArticyObjectWithVertices>(child);
		if (objWithVertices)
		{
			TArray<FVector2D> vertices = objWithVertices->GetVertices();
			FRect childBounds = FRect::GetBounds(&vertices);
			TArray<FVector2D> correctedPoints;

			// articy and unreal have a different y axis, so we have to take care of that and also incorporating the pixels to units conversion
			for (int i = 0; i < vertices.Num(); ++i)
			{
				FVector2D vec = vertices[i];
				FVector2D v(vec.X / PixelsToUnits, (childBounds.GetYMax() - vec.Y) / PixelsToUnits);
				correctedPoints.Add(v);
			}
			childBounds = FRect::GetBounds(&correctedPoints);
			overallBounds = FRect::Union(overallBounds, childBounds);
		}
	}

	// and now we can create all the elements inside the location
	auto location = Cast<UArticyObject>(Location);
	float zIndex = 0;
	GenerateChildren(WorldContext, location, PixelsToUnits, ObjectComponentMap, overallBounds, &zIndex, WorldContext, BackgroundLayer);


#endif // WITH_EDITOR
}

/* This method is called for every articy object in the location, starting with the location itself*/
void ULocationGenerator::GenerateChildren(AActor* Parent, UArticyObject* Child, float PixelsToUnits, const TMap<TSubclassOf<class UArticyBaseObject>, TSubclassOf<class UActorComponent>> ObjectComponentMap, FRect OverallBounds, float* YIndex, AActor* WorldContext, /*OUT*/ UManiacManfredLocationImage*& BackgroundLayer)
{
#if WITH_EDITOR

	auto articyDatabase = UArticyDatabase::Get(WorldContext);
	TArray<TWeakObjectPtr<UArticyObject>> children = Child->GetChildren();

	// The children are usually unsorted, but in this case we have to worry about proper ordering regarding depth sorting
	children.Sort([](const TWeakObjectPtr<UArticyObject>& ObjA, const TWeakObjectPtr<UArticyObject>& ObjB)
	{
		auto objWithZIndexA = Cast<IArticyObjectWithZIndex>(ObjA.Get());
		auto objWithZIndexB = Cast<IArticyObjectWithZIndex>(ObjB.Get());
		if (!objWithZIndexA || !objWithZIndexB)
			return false;
		return objWithZIndexA->GetZIndex() < objWithZIndexB->GetZIndex();
	});

	for (auto child : children)
	{
		APaperSpriteActor* createdChildActor;
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::Assert);

		FRect bounds = FRect();
		bool bBoundsSet = false;
		bool bHasZoneScript = false;

#pragma region Create new Actor for our location child

		// to keep it simple we instantiate every actor as a paper sprite actor, because in most cases we need them anyway
		FTransform transform(FQuat::Identity, FVector::OneVector, FVector::OneVector);
		createdChildActor = World->SpawnActorDeferred<APaperSpriteActor>(APaperSpriteActor::StaticClass(), transform, Parent);
		createdChildActor->SetActorLabel(GetNameForActor(child.Get()).ToString());
		createdChildActor->SetFolderPath(TEXT("GeneratedObjects"));
		createdChildActor->SetActorScale3D(FVector::OneVector);
		createdChildActor->OnConstruction(createdChildActor->GetTransform());
		createdChildActor->FinishSpawning(createdChildActor->GetTransform());
		createdChildActor->GetRenderComponent()->SetMobility(EComponentMobility::Stationary);
		createdChildActor->GetRenderComponent()->SetTranslucentSortPriority(*YIndex);

		// then we add an articyReference to it, storing the articy object that this new actor represents with it
		UArticyReference* articyReference = NewObject<UArticyReference>(createdChildActor);
		createdChildActor->AddInstanceComponent(articyReference);
		articyReference->SetReference(child.Get());

#pragma endregion


#pragma region Attach Behaviours By Template to the new actor

		for (auto& Pair : ObjectComponentMap)
		{
			/* We created in Blueprint an object-component map, which uses the type of an articy object as key
			*  and the component that should be attached to the actor representation of this articy object.
			*  (e.g. articy objects of type ConditionalZone should have an ClickableZone component attached)
			*/

			if (child->IsA(Pair.Key))
			{				
				auto component = NewObject<UActorComponent>(createdChildActor, Pair.Value.Get());
				createdChildActor->AddInstanceComponent(component);

				bHasZoneScript = component->GetClass()->GetName().Contains("ClickableZone");
			}
		}

#pragma endregion


#pragma region Create and setup sprite for location images

		if (auto objectWithLocationImage = Cast<UManiacManfredLocationImage>(child))
		{
			// load sprite and add to renderer component
			UArticyAsset* imageAsset = Cast<UArticyAsset>(articyDatabase->GetObject(objectWithLocationImage->ImageAsset));
			if (imageAsset && imageAsset->Category == EArticyAssetCategory::Image)
			{
				// load texture
				auto texture = Cast<UTexture2D>(imageAsset->LoadAsTexture());
				// adjust tiling method of the texture
				texture->AddressX = TextureAddress::TA_Clamp;
				texture->AddressY = TextureAddress::TA_Clamp;
				texture->MarkPackageDirty();

				// create sprite from texture
				FSpriteAssetInitParameters initParams;
				initParams.SetTextureAndFill(texture);
				UPaperSprite* sprite = Cast<UPaperSprite>(NewObject<UPaperSprite>(createdChildActor));
				sprite->SetPivotMode(ESpritePivotMode::Bottom_Left, FVector2D::ZeroVector);
				sprite->InitializeSprite(initParams);

				// apply sprite to actor
				createdChildActor->GetRenderComponent()->SetSprite(sprite);			
			}

			// if this location image is a background image, we store its reference in order
			// to return it and pass it to the BackgroundImageHandler Component later in Blueprint
			auto displayName = Cast<IArticyObjectWithDisplayName>(child);
			if (displayName && displayName->GetDisplayName().ToString() == "Background layer")
				BackgroundLayer = objectWithLocationImage;
		}

#pragma endregion

#pragma region Create a Collider if it a zone (has verices and bHasZoneScript is true)

		/* We calculate the bounds for every object with vertices, even if we don't want to attach a collider to it.
		*  So we make sure, that we don't run into issues when we adjust the transformations
		*/

		auto objectWithVertices = Cast<IArticyObjectWithVertices>(child);
		if (objectWithVertices)
		{
			auto vertices = objectWithVertices->GetVertices();

			// we calculate the overall bounds of this polygon
			bounds = FRect::GetBounds(&vertices);

			// ajdust pixel scaling
			TArray<FVector2D> colliderPoints;
			for (int i = 0; i < vertices.Num(); ++i)
			{
				auto vec = vertices[i];
				
				auto v = FVector2D(vec.X / PixelsToUnits, vec.Y / PixelsToUnits);
				colliderPoints.Add(v);
			}

			if (bHasZoneScript)
			{
				// if it a zone, we create a new sprite, set the collision points on it and apply it to the paper sprite actor
				FSpriteAssetInitParameters initParams;
				initParams.SetPixelsPerUnrealUnit(1);
				UPaperSprite* sprite = Cast<UPaperSprite>(NewObject<UPaperSprite>(createdChildActor));

				SetSpritePolygonCollider(sprite, colliderPoints);

				sprite->InitializeSprite(initParams);
				sprite->MarkPackageDirty();
				
				createdChildActor->GetRenderComponent()->SetSprite(sprite);			
			}
			else
			{
				// we need the bounds again, if this object does not only contain vertices, but also represents an image
				bounds = FRect::GetBounds(&colliderPoints);
				// if the actor represents an image instead of a zone, we disable the collision on it.
				createdChildActor->GetRenderComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			bBoundsSet = true;
		}

#pragma endregion

#pragma region Adjust the objects transformations

		auto objectWithTransform = Cast<IArticyObjectWithTransform>(child);
		if (objectWithTransform)
		{
			// if the bounds are not set up to now, we generate them from the sprite size
			if (!bBoundsSet)
			{
				auto sprite = createdChildActor->GetRenderComponent()->GetSprite();
				if (sprite)
				{
					bounds.w = sprite->GetSourceSize().Y / PixelsToUnits;
					bounds.h = sprite->GetSourceSize().X / PixelsToUnits;
				}
			}

			// update transform
			auto translation = objectWithTransform->GetTransform()->Translation;
			if (bHasZoneScript)
			{
				// change the position
				createdChildActor->SetActorLocation(FVector(translation.X / PixelsToUnits, *YIndex,
					bounds.GetYMax() + (OverallBounds.h - bounds.h - (translation.Y / PixelsToUnits))), false);
			}
			else
			{
				createdChildActor->SetActorLocation(FVector(translation.X / PixelsToUnits, *YIndex,
					OverallBounds.h - bounds.h - (translation.Y / PixelsToUnits)), false);
			}

		}

#pragma endregion
		
		*YIndex += 0.2;
		// some objects inside a location, can have children themselves, so we just recursively call GenerateChildren again
		GenerateChildren(createdChildActor, child.Get(), PixelsToUnits, ObjectComponentMap, OverallBounds, YIndex, WorldContext, BackgroundLayer);
	}

#endif // WITH_EDITOR
}

void ULocationGenerator::SetSpritePolygonCollider(UPaperSprite* Sprite, TArray<FVector2D> Vertices)
{
#if WITH_EDITOR
	// here we use Unreals reflection to set the collider on a sprite

	if (FProperty* collisionGeometryProp = Sprite->GetClass()->FindPropertyByName("CollisionGeometry"))
	{
		void* structAddress = collisionGeometryProp->ContainerPtrToValuePtr<void>(Sprite);

		if (FStructProperty* structProp = CastField<FStructProperty>(collisionGeometryProp))
		{
			UScriptStruct* scriptStruct = structProp->Struct;
		
			if (FArrayProperty* shapesProp = CastField<FArrayProperty>(scriptStruct->FindPropertyByName("Shapes")))
			{
				FSpriteGeometryCollection* collisionGeometryPtr = shapesProp->ContainerPtrToValuePtr<FSpriteGeometryCollection>(structAddress);

				FSpriteGeometryShape polygonCollider;
				polygonCollider.ShapeType = ESpriteShapeType::Polygon;
				polygonCollider.Vertices = Vertices;

				collisionGeometryPtr->GeometryType = ESpritePolygonMode::FullyCustom;
				collisionGeometryPtr->Shapes.Empty();
				collisionGeometryPtr->Shapes.Add(polygonCollider);

				collisionGeometryPtr->ConditionGeometry();
			}

			scriptStruct->PostEditChange();
		}
	}
#endif // WITH_EDITOR
}