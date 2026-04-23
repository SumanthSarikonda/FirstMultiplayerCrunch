// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/BaseGameplayAbility.h"
#include "GAP_Launched.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Launched : public UBaseGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGAP_Launched();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	static FGameplayTag GetLaunchedAbilityActivationTag();
};
