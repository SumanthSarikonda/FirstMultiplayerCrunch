// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/BaseGameplayAbility.h"
#include "GA_Combo.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Combo : public UBaseGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Combo();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	static FGameplayTag GetComboChangedEventTag();
	static FGameplayTag GetComboChangedEventEndTag();
	static FGameplayTag GetComboTargetEventTag();
	
private:
	void SetupWaitInputPress();
	
	UFUNCTION()
	void HandleInputPress(float TimeWaited);
	
	void TryCommitCombo();
	
	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> DefaultDamageEffect;
	
	UPROPERTY(EditAnywhere, Category = "Gameplay Effect")
	TMap<FName, TSubclassOf<UGameplayEffect>> DamageEffectMap;
	
	TSubclassOf<UGameplayEffect> GetDamageEffectForCurrentCombo() const;
	
	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* ComboMontage;
	
	UFUNCTION()
	void ComboChangeEventReceived(FGameplayEventData Data);
	
	UFUNCTION()
	void DoDamage(FGameplayEventData Data);
	
	FName NextComboName; 
};
