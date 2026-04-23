// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CHeroAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
 	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
 	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
 	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
 	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
/**
 * 
 */
UCLASS()
class UCHeroAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
    ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, Intelligence)
    ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, Strength)
    ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, Experience)
    ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, PrevLevelExperience)
    ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, NextLevelExperience)
    ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, Level)
	ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, UpgradePoint)
    ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, MaxLevel)
	ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, MaxLevelExp)
    ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, Gold)
	ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, StrengthGrowthRate)
	ATTRIBUTE_ACCESSORS(UCHeroAttributeSet, IntelligenceGrowthRate)
	virtual void GetLifetimeReplicatedProps( TArray< class FLifetimeProperty > & OutLifetimeProps ) const override;
private:
	UPROPERTY(ReplicatedUsing = OnRep_Intelligence)
	FGameplayAttributeData Intelligence;

	UPROPERTY(ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	
	UPROPERTY(ReplicatedUsing = OnRep_Experience)
	FGameplayAttributeData Experience;
	
	UPROPERTY()
	FGameplayAttributeData StrengthGrowthRate;
	
	UPROPERTY()
	FGameplayAttributeData IntelligenceGrowthRate;

	UPROPERTY(ReplicatedUsing = OnRep_PrevLevelExperience)
	FGameplayAttributeData PrevLevelExperience;

	UPROPERTY(ReplicatedUsing = OnRep_NextLevelExperience)
	FGameplayAttributeData NextLevelExperience;

	UPROPERTY(ReplicatedUsing = OnRep_Level)
	FGameplayAttributeData Level;
	
	UPROPERTY(ReplicatedUsing = OnRep_UpgradePoint)
	FGameplayAttributeData UpgradePoint;
	
	UPROPERTY(ReplicatedUsing = OnRep_MaxLevel)
	FGameplayAttributeData MaxLevel;
	
	UPROPERTY(ReplicatedUsing = OnRep_MaxLevelExp)
	FGameplayAttributeData MaxLevelExp;

	UPROPERTY(ReplicatedUsing = OnRep_Gold)
	FGameplayAttributeData Gold;

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Experience(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_PrevLevelExperience(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_NextLevelExperience(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_UpgradePoint(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxLevel(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MaxLevelExp(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Gold(const FGameplayAttributeData& OldValue);
};
