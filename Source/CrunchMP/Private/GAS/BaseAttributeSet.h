// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class UBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, CachedHealthPercent)
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, CachedManaPercent)
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Mana)
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxMana)
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, AttackDamage)
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Armor)
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MoveSpeed)
	virtual void GetLifetimeReplicatedProps( TArray< class FLifetimeProperty > & OutLifetimeProps ) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override; 
	
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;
	
	void ReScaleHealth();
	void ReScaleMana();
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	
	UPROPERTY(ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	
	UPROPERTY(ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	
	UPROPERTY(ReplicatedUsing = OnRep_AttackDamage)
	FGameplayAttributeData AttackDamage;
	
	UPROPERTY(ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;
	
	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	
	UPROPERTY()
	FGameplayAttributeData CachedHealthPercent;
	
	UPROPERTY()
	FGameplayAttributeData CachedManaPercent;
	
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_AttackDamage(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
};
