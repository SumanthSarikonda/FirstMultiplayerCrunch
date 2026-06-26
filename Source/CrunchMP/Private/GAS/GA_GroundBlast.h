// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/BaseGameplayAbility.h"
#include "CGameplayAbilityTypes.h"
#include "GA_GroundBlast.generated.h"

/**
 * 
 */
UCLASS()
class UGA_GroundBlast : public UBaseGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_GroundBlast();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Cue")
	FGameplayTag BlastGameplayCueTag;
	
	UPROPERTY(EditAnywhere, Category = "Targetting")
	float TargetAreaRadius = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "Damage")
	FGenericDamageEffectDef DamageEffectDef;
	
	UPROPERTY(EditAnywhere, Category = "Targetting")
	float TargetTraceRange = 2000.f;
	
	UPROPERTY(EditAnywhere, Category = "Targetting")
	TSubclassOf<class ATargetActor_GroundPick> TargetActorClass;
	
	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* TargetingMontage;
	
	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* CastMontage;
	
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
	
};
