// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/AbilityGauge.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/CHeroAttributeSet.h"
#include "GAS/AbilitySystemTags.h"
#include "GAS/BaseAttributeSet.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Widgets/AbilityToolTip.h"

void UAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct(); 
	
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnerASC)
	{
		OwnerASC->AbilityCommittedCallbacks.AddUObject(this, &UAbilityGauge::AbilityComitted);
		OwnerASC->AbilitySpecDirtiedCallbacks.AddUObject(this, &UAbilityGauge::AbilitySpecUpdated);
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetUpgradePointAttribute()).AddUObject(this, &UAbilityGauge::UpgradePointUpdated);
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetManaAttribute()).AddUObject(this, &UAbilityGauge::ManaUpdated);
		
		bool bFound = false;
		float UpgradePoint = OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetUpgradePointAttribute(), bFound);
		if (bFound)
		{
			FOnAttributeChangeData ChangeData;
			ChangeData.NewValue = UpgradePoint;
			UpgradePointUpdated(ChangeData);
		}
	}
	const FGameplayAbilitySpec* Spec = GetAbilitySpec();
	if (Spec)
	{
		AbilitySpecUpdated(*Spec);
	}
	
	OwnerAbilitySystemComponent = OwnerASC;
	WholeNumberFormattingOptions.MaximumFractionalDigits = 0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits = 2;
}

void UAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	AbilityCDO = Cast<UGameplayAbility>(ListItemObject);
	
	float CooldownDuration = AbilityTags::GetStaticCooldownDurationForAbility(AbilityCDO);
	float Cost = AbilityTags::GetStaticCostForAbility(AbilityCDO);
	
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	CostText->SetText(FText::AsNumber(Cost));
	LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(AbilityLevelParamName, 0);
	
}

void UAbilityGauge::ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData)
{
	if (Icon && WidgetData)
	{
		Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParamName, WidgetData->Icon.LoadSynchronous());
		CreateToolTipWidget(WidgetData);
	}
}

void UAbilityGauge::CreateToolTipWidget(const FAbilityWidgetData* AbilityWidgetData)
{
	if (!AbilityWidgetData || !AbilityToolTipClass)
		return;
	
	UAbilityToolTip* InstantiatedToolTip = CreateWidget<UAbilityToolTip>(GetOwningPlayer(), AbilityToolTipClass);
	if (InstantiatedToolTip)
	{
		float CooldownDuration = AbilityTags::GetStaticCooldownDurationForAbility(AbilityCDO);
		float Cost = AbilityTags::GetStaticCostForAbility(AbilityCDO);
		
		InstantiatedToolTip->SetAbilityInfo(AbilityWidgetData->AbilityName, AbilityWidgetData->Icon.LoadSynchronous(), AbilityWidgetData->Description, CooldownDuration, Cost);
		
		SetToolTip(InstantiatedToolTip);
	}
}

void UAbilityGauge::AbilityComitted(UGameplayAbility* Ability)
{
	if (Ability->GetClass()->GetDefaultObject() == AbilityCDO)
	{
		float CooldownTimeRemaining = 0.f;
		float CooldownDuration = 0.f;
		
		Ability->GetCooldownTimeRemainingAndDuration(Ability->GetCurrentAbilitySpecHandle(), Ability->GetCurrentActorInfo(), CooldownTimeRemaining, CooldownDuration);
		
		StartCooldown(CooldownTimeRemaining, CooldownDuration);
	}
}

void UAbilityGauge::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	CachedCooldownDuration = CooldownDuration;
	CachedCooldownTimeRemaining = CooldownTimeRemaining;
	
	CooldownCountText->SetVisibility(ESlateVisibility::Visible);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UAbilityGauge::CooldownFinished, CachedCooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerUpdateHandle, this, &UAbilityGauge::UpdateCooldown, CooldownUpdatedInterval, true, 0);
}

void UAbilityGauge::CooldownFinished()
{
	CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamName, 1.f);
}

void UAbilityGauge::UpdateCooldown()
{
	CachedCooldownTimeRemaining -= CooldownUpdatedInterval;
	FNumberFormattingOptions* FormattingOptions = CachedCooldownTimeRemaining > 1 ? &WholeNumberFormattingOptions : &TwoDigitNumberFormattingOptions;
	CooldownCountText->SetText(FText::AsNumber(CachedCooldownTimeRemaining, FormattingOptions));
	
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamName, 1.0f - CachedCooldownTimeRemaining / CachedCooldownDuration);
}

const FGameplayAbilitySpec* UAbilityGauge::GetAbilitySpec()
{
	if (!OwnerAbilitySystemComponent)
		return nullptr;
	
	if (!AbilityCDO)
		return nullptr;
	
	if (!CachedAbilitySpecHandle.IsValid())
	{
		FGameplayAbilitySpec* FoundAbilitySpec = OwnerAbilitySystemComponent->FindAbilitySpecFromClass(AbilityCDO->GetClass());
		CachedAbilitySpecHandle = FoundAbilitySpec->Handle;
		return FoundAbilitySpec;
	}
	
	return OwnerAbilitySystemComponent->FindAbilitySpecFromHandle(CachedAbilitySpecHandle);
}

void UAbilityGauge::AbilitySpecUpdated(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability != AbilityCDO)
		return;
	
	bIsAbilityLearned = AbilitySpec.Level > 0;
	LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(AbilityLevelParamName, AbilitySpec.Level);
	UpdateCanCast();
	
	float NewCooldownDuration = AbilityTags::GetCooldownDurationFor(AbilitySpec.Ability, *OwnerAbilitySystemComponent, AbilitySpec.Level);
	float NewCost = AbilityTags::GetManaCostFor(AbilitySpec.Ability, *OwnerAbilitySystemComponent, AbilitySpec.Level);
	
	CooldownDurationText->SetText(FText::AsNumber(NewCooldownDuration));
	CostText->SetText(FText::AsNumber(NewCost));
}

void UAbilityGauge::UpdateCanCast()
{
	const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpec();
	bool bCanCast = bIsAbilityLearned;
	if (AbilitySpec)
	{
		if (OwnerAbilitySystemComponent && !AbilityTags::CheckAbilityCost(*AbilitySpec, *OwnerAbilitySystemComponent))
		{
			bCanCast = false;
		}
	}
	
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CanCastAbilityParamName, bCanCast ? 1.f : 0.f);
}

void UAbilityGauge::UpgradePointUpdated(const FOnAttributeChangeData& Data)
{
	bool HasUpgradePoint = Data.NewValue > 0;
	const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpec();
	if (AbilitySpec)
	{
		if (AbilityTags::IsAbilityAtMaxLevel(*AbilitySpec))
		{
			Icon->GetDynamicMaterial()->SetScalarParameterValue(UpgradePointAvailableParamName, 0);
			return;
		}
	}
	
	return Icon->GetDynamicMaterial()->SetScalarParameterValue(UpgradePointAvailableParamName, HasUpgradePoint ? 1.f : 0.f);
}

void UAbilityGauge::ManaUpdated(const FOnAttributeChangeData& Data)
{
	UpdateCanCast();
}
