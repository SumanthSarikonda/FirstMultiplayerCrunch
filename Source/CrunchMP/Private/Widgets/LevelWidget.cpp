// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/LevelWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/CHeroAttributeSet.h"

void ULevelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	NumberFormattingOptions.SetMaximumFractionalDigits(0);
	
	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (!OwnerPawn)
		return;
	
	UAbilitySystemComponent* OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);
	if (!OwnerAbilitySystemComponent)
		return;

	OwnerAsc = OwnerAbilitySystemComponent;
	
	UpdateGauge(FOnAttributeChangeData());
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this, &ULevelWidget::UpdateGauge);
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetNextLevelExperienceAttribute()).AddUObject(this, &ULevelWidget::UpdateGauge);
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetPrevLevelExperienceAttribute()).AddUObject(this, &ULevelWidget::UpdateGauge);
	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetLevelAttribute()).AddUObject(this, &ULevelWidget::UpdateGauge);
}

void ULevelWidget::UpdateGauge(const FOnAttributeChangeData& Data)
{
	bool bFound;
	float CurrentExp = OwnerAsc->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(), bFound);
	if (!bFound)
		return;
	float NextLevelExp = OwnerAsc->GetGameplayAttributeValue(UCHeroAttributeSet::GetNextLevelExperienceAttribute(), bFound);
	if (!bFound)
		return;
	float PreLevelExp = OwnerAsc->GetGameplayAttributeValue(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(), bFound);
	if (!bFound)
		return;
	float CurrentLevel = OwnerAsc->GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(), bFound);
	if (!bFound)
		return;
	
	LevelText->SetText(FText::AsNumber(CurrentLevel, &NumberFormattingOptions));
	
	float Progress = CurrentExp - PreLevelExp;
	float LevelExpAmt = NextLevelExp - PreLevelExp;
	
	float Percent = Progress / LevelExpAmt;
	
	if (NextLevelExp == 0)
	{
		Percent = 1;
	}
	
	if (LevelProgressImage)
	{
		LevelProgressImage->GetDynamicMaterial()->SetScalarParameterValue(PercentMaterialParamName, Percent);
	}
}
