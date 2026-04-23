// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_AbilitySystemGenerics.generated.h"

class UGameplayEffect;
class UGameplayAbility;
/**
 * 
 */
UCLASS()
class UPA_AbilitySystemGenerics : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	FORCEINLINE TSubclassOf<UGameplayEffect> GetFullStatEffect() const {return FullStats; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDeathEffect() const {return DeathEffect; }
	FORCEINLINE const TArray<TSubclassOf<UGameplayEffect>>& GetStartingEffect() const {return StartingEffects; }
	FORCEINLINE const TArray<TSubclassOf<UGameplayAbility>>& GetPassiveAbilities() const {return PassiveAbilities; }
	FORCEINLINE const UDataTable* GetBaseStatsDT() const {return BaseStatsDT; }
	const FRealCurve* GetExpCurve() const;
	
private:
	
	UPROPERTY(EditAnywhere, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStats;
	
	UPROPERTY(EditAnywhere, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;
	
	UPROPERTY(EditAnywhere, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> StartingEffects;
	
	UPROPERTY(EditAnywhere, Category = "Gameplay Abilities")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;
	
	UPROPERTY(EditAnywhere, Category = "Base Stats")
	UDataTable* BaseStatsDT;
	
	UPROPERTY(EditAnywhere, Category = "Level")
	FName ExpRowName = "ExpNeededToReachLevel";
	
	UPROPERTY(EditAnywhere, Category = "Level")
	UCurveTable* ExpCT;
};
