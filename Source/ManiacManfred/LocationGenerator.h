// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Paper2DClasses.h"
#include "ArticyBaseInclude.h"
#include "ArticyGenerated/ManiacManfredArticyTypes.h"
#include "LocationGenerator.generated.h"

USTRUCT()
struct MANIACMANFRED_API FRect
{
	GENERATED_BODY()

public:

		float x;
		float y;
		float w;
		float h;

	float GetXMax()
	{
		return w + x;
	};

	void SetXMax(float Value)
	{
		w = Value - x;
	};

	float GetYMax()
	{
		return h + y;
	};

	void SetYMax(float Value)
	{
		h = Value - y;
	};

	static FRect GetBounds(const TArray<FVector2D>* Vertices)
	{
		FRect rc = FRect();
		rc.x = INFINITY;
		rc.y = INFINITY;
		rc.w = 0;
		rc.h = 0;

		float right = 0;
		float bottom = 0;
		for (int i = 0; i < Vertices->Num(); ++i)
		{
			auto vec = (*Vertices)[i];

			rc.x = FMath::Min(rc.x, vec.X);
			rc.y = FMath::Min(rc.y, vec.Y);

			right = FMath::Max(right, vec.X);
			bottom = FMath::Max(bottom, vec.Y);
		}

		rc.SetXMax(right);
		rc.SetYMax(bottom);

		return rc;
	}

	static FRect Union(FRect A, FRect B)
	{
		float x1 = FMath::Min(A.x, B.x);
		float x2 = FMath::Max(A.x + A.w, B.x + B.w);
		float y1 = FMath::Min(A.y, B.y);
		float y2 = FMath::Max(A.y + A.h, B.y + B.h);

		FRect rc;
		rc.x = x1;
		rc.y = y1;
		rc.w = x2 - x1;
		rc.h = y2 - y1;

		return rc;
	}
};

/**
 * 
 */
UCLASS()
class MANIACMANFRED_API ULocationGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	static void GenerateLocation(UManiacManfredLocation* Location, float PixelsToUnits, const TMap<TSubclassOf<class UArticyBaseObject>, TSubclassOf<class UActorComponent>> ObjectComponentMap, AActor* WorldContext, /*OUT*/ UManiacManfredLocationImage*& BackgroundLayer);

private:

	static void GenerateChildren(AActor* Parent, UArticyObject* Child, float PixelsToUnits, const TMap<TSubclassOf<class UArticyBaseObject>, TSubclassOf<class UActorComponent>> ObjectComponentMap, FRect OverallBounds, float* ZIndex, AActor* WorldContext, /*OUT*/ UManiacManfredLocationImage*& BackgroundLayer);
	static void SetSpritePolygonCollider(UPaperSprite* Sprite, TArray<FVector2D> Vertices);
};
