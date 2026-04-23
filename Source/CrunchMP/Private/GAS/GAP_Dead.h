// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/BaseGameplayAbility.h"
#include "GAP_Dead.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Dead : public UBaseGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGAP_Dead();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Reward")
	float RewardRange = 1000.f;
	
	UPROPERTY(EditAnywhere, Category = "Reward")
	float BaseExpReward  = 200.f;
	
	UPROPERTY(EditAnywhere, Category = "Reward")
	float BaseGoldReward  = 200.f;
	
	UPROPERTY(EditAnywhere, Category = "Reward")
	float ExpRewardPerExp  = 0.1f;
	
	UPROPERTY(EditAnywhere, Category = "Reward")
	float GoldRewardPerExp  = 0.05f;
	
	UPROPERTY(EditAnywhere, Category = "Reward")
	float KillerRewardPortion  = 0.5f;
	
	TArray<AActor*> GetRewardTargets() const;
	
	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	TSubclassOf<UGameplayEffect> RewardEffect;
};
