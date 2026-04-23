// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "BaseAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UBaseAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	UBaseAbilitySystemComponent();
	void InitializeBaseAttributes();
	void ServerSideInit();
	void ApplyAllStats();
	const TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const;
	bool IsAtMaxLevel() const;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UpgradeAbilityWithInputId(EAbilityInputID InputID);
	
	UFUNCTION(Server, Reliable)
	void Client_AbilitySpecLevelUpdated(FGameplayAbilitySpecHandle Handle, int NewLevel);
	
private:
	void ApplyStartingEffects();
	void GrantInitialAbilities();
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level = 1);
	void HealthUpdate(const FOnAttributeChangeData& ChangeData);
	void ManaUpdate(const FOnAttributeChangeData& ChangeData);
	void ExpUpdate(const FOnAttributeChangeData& ChangeData);
	
	UPROPERTY(EditAnywhere, Category = "Gameplay Abilities")
	TMap<EAbilityInputID , TSubclassOf<UGameplayAbility>> Abilities;
	
	UPROPERTY(EditAnywhere, Category = "Gameplay Abilities")
	TMap<EAbilityInputID , TSubclassOf<UGameplayAbility>> BasicAbilities;
	
	UPROPERTY(EditAnywhere, Category = "Gameplay Abilities")
	class UPA_AbilitySystemGenerics* AbilitySystemGenerics;
};
