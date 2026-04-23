// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AbilitySystemTags.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

namespace AbilityTags 
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_Player, "Role.Player", "Player");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(BasicAttack, "Ability.BasicAttack", "BasicAttack");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Dead, "Status.Dead", "Death");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Stun, "Status.Stun", "Stun");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_HealthFull, "Status.Health.Full", "FullHealth");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_HealthEmpty, "Status.Health.Empty", "EmptyHealth");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_ManaFull, "Status.Mana.Full", "FullHealth");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_ManaEmpty, "Status.Mana.Empty", "EmptyHealth");
    
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Aim, "Status.Aim", "Aim");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_CameraShake, "GameplayCue.CameraShake", "CameraShake");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(BasicAttack_Pressed, "Ability.BasicAttack.Pressed", "BasicAttack Pressed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(BasicAttack_Released, "Ability.BasicAttack.Released", "BasicAttack Released");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AttributeExperience, "Attr.Experience", "AttributeExperience");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AttributeGold, "Attr.Gold", "AttributeGold");
       
}

bool AbilityTags::IsPlayer(const AActor* ActorToCheck)
{
	const IAbilitySystemInterface* ActorISA = Cast<IAbilitySystemInterface>(ActorToCheck);
	if (ActorISA)
	{
		UAbilitySystemComponent* ActorASC = ActorISA->GetAbilitySystemComponent();
		if (ActorASC)
		{
			return ActorASC->HasMatchingGameplayTag(AbilityTags::Role_Player);
		}
	}
	return false;
}

bool AbilityTags::IsAbilityAtMaxLevel(const FGameplayAbilitySpec& Spec)
{
	return Spec.Level >= 4;
}


float AbilityTags::GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
		return 0.f;
    
	const UGameplayEffect* CooldownEffect = Ability->GetCooldownGameplayEffect();
	if (!CooldownEffect)
		return 0.f;
    
	float CooldownDuration = 0.f;
    
	CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(1, CooldownDuration);
	return CooldownDuration;
}

float AbilityTags::GetStaticCostForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
		return 0.f;
    
	const UGameplayEffect* CostEffect = Ability->GetCostGameplayEffect();
	if (!CostEffect || CostEffect->Modifiers.Num() == 0)
		return 0.f;
    
	float Cost = 0.f;
	CostEffect->Modifiers[0].ModifierMagnitude.GetStaticMagnitudeIfPossible(1, Cost);  
	return FMath::Abs(Cost);
}

bool AbilityTags::CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& Asc)
{
	const UGameplayAbility* AbilityCDO = AbilitySpec.Ability;
	if (AbilityCDO)
	{
		return  AbilityCDO->CheckCost(AbilitySpec.Handle, Asc.AbilityActorInfo.Get());
	}
	
	return false;
}

float AbilityTags::GetManaCostFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& Asc,
	int AbilityLevel)
{
	float ManaCost = 0.f;
	if (AbilityCDO)
	{
		UGameplayEffect* CostEffect = AbilityCDO->GetCostGameplayEffect();
		if (CostEffect)
		{
			FGameplayEffectSpecHandle EffectSpec = Asc.MakeOutgoingSpec(CostEffect->GetClass(), AbilityLevel, Asc.MakeEffectContext());
			CostEffect->Modifiers[0].ModifierMagnitude.AttemptCalculateMagnitude(*EffectSpec.Data.Get(), ManaCost);
		}
	}
	
	return FMath::Abs(ManaCost);
}

float AbilityTags::GetCooldownDurationFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& Asc,
	int AbilityLevel)
{
	float CooldownDuration = 0.f;
	if (AbilityCDO)
	{
		UGameplayEffect* CooldownEffect = AbilityCDO->GetCooldownGameplayEffect();
		if (CooldownEffect)
		{
			FGameplayEffectSpecHandle EffectSpec = Asc.MakeOutgoingSpec(CooldownEffect->GetClass(), AbilityLevel, Asc.MakeEffectContext());
			CooldownEffect->DurationMagnitude.AttemptCalculateMagnitude(*EffectSpec.Data.Get(), CooldownDuration);
		}
	}
	
	return FMath::Abs(CooldownDuration);
}

float AbilityTags::GetCooldownRemaining(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& Asc)
{
	if (!AbilityCDO)
		return 0;
	
	UGameplayEffect* CooldownEffect = AbilityCDO->GetCooldownGameplayEffect();
	if (!CooldownEffect)
		return 0;
	
	FGameplayEffectQuery CooldownEffectQuery;
	CooldownEffectQuery.EffectDefinition = CooldownEffect->GetClass();
	
	float CooldownRemaining = 0.f;
	FJsonSerializableArrayFloat CooldownTimeRemainings = Asc.GetActiveEffectsTimeRemaining(CooldownEffectQuery);
	
	for (float Remaining : CooldownTimeRemainings)
	{
		if (Remaining > CooldownRemaining)
		{
			CooldownRemaining = Remaining;
		}
	}
	
	return CooldownRemaining;
}
