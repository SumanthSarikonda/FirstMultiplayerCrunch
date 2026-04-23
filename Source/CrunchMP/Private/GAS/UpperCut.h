// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/BaseGameplayAbility.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "UpperCut.generated.h"

/**
 * 
 */
UCLASS()
class UUpperCut : public UBaseGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UUpperCut();
	
private:
	UPROPERTY(EditAnywhere, Category = "Combo")
	TMap<FName, FGenericDamageEffectDef> ComboDamageMap;
	
	UPROPERTY(EditAnywhere, Category = "Launch")
	TSubclassOf<UGameplayEffect> LaunchDamageEffect;
	
	UPROPERTY(EditAnywhere, Category = "Launch")
	float UpperCutLaunchSpeed = 1000.f;
	
	UPROPERTY(EditAnywhere, Category = "Launch")
	float UpperCutComboStableSpeed = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* UpperCutMontage;
	
	static FGameplayTag GetUpperCutLaunchTag();
	
	const FGenericDamageEffectDef* GetDamageEffectDefForCurrentCombo() const;
	
	UFUNCTION()
	void StartLaunching(FGameplayEventData EventData);
	
	UFUNCTION()
	void HandleComboChangeEvent(FGameplayEventData EventData);
	
	UFUNCTION()
	void HandleComboCommitEvent(FGameplayEventData EventData);
	
	UFUNCTION()
	void HandleComboDamageEvent(FGameplayEventData EventData);
	
	FName NextComboName;
};
