// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/AbilityListView.h"
#include "Abilities/GameplayAbility.h"
#include "Widgets/AbilityGauge.h"

void UAbilityListView::ConfigureAbilities(const TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities)
{
	OnEntryWidgetGenerated().AddUObject(this, &UAbilityListView::AbilityGaugeDegenrated);
	for (const TPair<EAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		AddItem(AbilityPair.Value.GetDefaultObject());
	}
}

void UAbilityListView::AbilityGaugeDegenrated(UUserWidget& Widget)
{
	UAbilityGauge* AbilityGauge = Cast<UAbilityGauge>(&Widget);
	
	if (AbilityGauge)
	{
		AbilityGauge->ConfigureWithWidgetData(FindWidgetData(AbilityGauge->GetListItem<UGameplayAbility>()->GetClass()));
	}
}

const struct FAbilityWidgetData* UAbilityListView::FindWidgetData(
	const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
	if (!AbilityClass)
		return nullptr;
	
	for (auto& AbilityWidgetDataPair: AbilityDataTable->GetRowMap())
	{
		const FAbilityWidgetData* WidgetData = AbilityDataTable->FindRow<FAbilityWidgetData>(AbilityWidgetDataPair.Key, "");
		if (WidgetData->AbilityClass ==  AbilityClass)
		{
			return WidgetData;
		}
	}
	return nullptr;
}
