// Fill out your copyright notice in the Description page of Project Settings.

#include "ManiacManfredUtility.h"

#include "Paper2DClasses.h"
#include "Engine/World.h"


/*
* Gets called to change the background image in the last level depending on the morale value without triggering Paper2D editor-only code.
* It may not be the best solution, but for this one usecase its sufficient as a quick and dirty workaround.
*/
void UManiacManfredUtility::ChangeSpriteFromTexture(APaperSpriteActor* Actor, UTexture* Texture)
{
	UPaperSprite* sprite = Actor->GetRenderComponent()->GetSprite();
	
	if (FObjectPropertyBase* spriteSourceProp = CastField<FObjectPropertyBase>(sprite->GetClass()->FindPropertyByName(TEXT("SourceTexture"))))
	{
		if (UTexture2D* spriteSourcePropPtr = spriteSourceProp->ContainerPtrToValuePtr<UTexture2D>(sprite))
		{
			UTexture2D* texture2D = Cast<UTexture2D>(Texture);

			spriteSourceProp->SetObjectPropertyValue(spriteSourcePropPtr, texture2D);
			Actor->GetRenderComponent()->MarkRenderStateDirty();

			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Could not change texture on %s."), *Actor->GetFullName());
}

void UManiacManfredUtility::MarkPackageDirty(UObject* Object)
{
	Object->MarkPackageDirty();
}
