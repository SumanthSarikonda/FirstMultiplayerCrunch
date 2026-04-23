// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OverHeadStatsGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"
#include "GAS/BaseAttributeSet.h"

void UOverHeadStatsGauge::ConfigureWithASC(class UAbilitySystemComponent* AbilitySystemComponent)
{
	
	if (AbilitySystemComponent)
	{
		HealthBar->BindToGameplayAttribute(AbilitySystemComponent, UBaseAttributeSet::GetHealthAttribute(), UBaseAttributeSet::GetMaxHealthAttribute());
		ManaBar->BindToGameplayAttribute(AbilitySystemComponent, UBaseAttributeSet::GetManaAttribute(), UBaseAttributeSet::GetMaxManaAttribute());
	}
}
