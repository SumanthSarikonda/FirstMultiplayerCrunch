// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GAP_Launched.h"
#include "GAS/AbilitySystemTags.h"

UGAP_Launched::UGAP_Launched()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = GetLaunchedAbilityActivationTag();
	
	ActivationBlockedTags.RemoveTag(AbilityTags::Status_Stun);
	AbilityTriggers.Add(TriggerData);
}

void UGAP_Launched::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	
	if (K2_HasAuthority())
	{
		PushSelf(TriggerEventData->TargetData.Get(0)->GetHitResult()->ImpactPoint);
		K2_EndAbility();
	}
}

FGameplayTag UGAP_Launched::GetLaunchedAbilityActivationTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Passive.Launch.Activate");
}
