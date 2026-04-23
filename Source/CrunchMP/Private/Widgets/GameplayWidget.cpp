// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameplayWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/BaseAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"
#include "Widgets/AbilityListView.h"
#include "GAS/BaseAttributeSet.h"

void UGameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	
	if (OwnerAbilitySystemComponent)
	{
		HealthBar->BindToGameplayAttribute(OwnerAbilitySystemComponent, UBaseAttributeSet::GetHealthAttribute(), UBaseAttributeSet::GetMaxHealthAttribute());
		ManaBar->BindToGameplayAttribute(OwnerAbilitySystemComponent, UBaseAttributeSet::GetManaAttribute(), UBaseAttributeSet::GetMaxManaAttribute());
	}
}

void UGameplayWidget::ConfigureAbilities(const TMap<EAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
	AbilityListView->ConfigureAbilities(Abilities);
}
