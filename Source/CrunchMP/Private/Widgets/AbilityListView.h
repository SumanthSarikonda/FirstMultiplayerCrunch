// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "AbilityListView.generated.h"

/**
 * 
 */
UCLASS()
class UAbilityListView : public UListView
{
	GENERATED_BODY()
	
public:
	void ConfigureAbilities(const TMap<EAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities);
	
private:
	UPROPERTY(EditAnywhere, Category = "Data")
	UDataTable* AbilityDataTable;
	
	void AbilityGaugeDegenrated(UUserWidget& Widget);
	
	const struct FAbilityWidgetData* FindWidgetData(const TSubclassOf<UGameplayAbility>& AbilityClass) const;
};
