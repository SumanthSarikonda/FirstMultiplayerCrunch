#pragma once

#include "NativeGameplayTags.h"
#include "Abilities/GameplayAbility.h"

class UGameplayEffect;
struct FGameplayAbilitySpec;
class UAbilitySystemComponent;

namespace  AbilityTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Role_Player)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(BasicAttack)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Dead)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Stun)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_HealthFull)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_HealthEmpty)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_ManaFull)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_ManaEmpty)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Aim)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Crosshair)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Focus)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Target_Updated)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_CameraShake)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(BasicAttack_Pressed)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(BasicAttack_Released)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeExperience)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeGold)
	
	bool IsActorDead(const AActor* ActorToCheck);
	bool IsPlayer(const AActor* ActorToCheck);
	bool ActorHasTag(const AActor* ActorToCheck, const FGameplayTag& Tag);
	bool IsAbilityAtMaxLevel(const FGameplayAbilitySpec& Spec);
	
	float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	float GetStaticCostForAbility(const UGameplayAbility* Ability);
	
	bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& Asc);
	bool CheckAbilityCostStatic(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& Asc);
	float GetManaCostFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& Asc, int AbilityLevel);
	float GetCooldownDurationFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& Asc, int AbilityLevel);
	float GetCooldownRemaining(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& Asc);
}
