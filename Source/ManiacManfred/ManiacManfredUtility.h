// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PaperSpriteActor.h"
#include "ManiacManfredUtility.generated.h"


UCLASS()
class MANIACMANFRED_API UManiacManfredUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Maniac Manfred Utility")
	static void ChangeSpriteFromTexture(APaperSpriteActor* Actor, UTexture* Texture);

	UFUNCTION(BlueprintCallable, Category = "Maniac Manfred Utility")
	static void MarkPackageDirty(UObject* Object);
};