// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "LevelWidget.generated.h"

/**
 * 
 */
UCLASS()
class ULevelWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName PercentMaterialParamName = "Percent";
	
	UPROPERTY(meta=(BindWidget))
	class UImage* LevelProgressImage;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* LevelText;
	
	FNumberFormattingOptions NumberFormattingOptions;
	
	const class UAbilitySystemComponent* OwnerAsc;
	
	void UpdateGauge(const FOnAttributeChangeData& Data);
	
};
