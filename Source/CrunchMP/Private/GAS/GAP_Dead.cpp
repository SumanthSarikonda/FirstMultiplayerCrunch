// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GAP_Dead.h"
#include "GAS/AbilitySystemTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/BaseAttributeSet.h"
#include "GAS/CHeroAttributeSet.h"
#include "Engine/OverlapResult.h"

UGAP_Dead::UGAP_Dead()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = AbilityTags::Status_Dead;
	
	AbilityTriggers.Add(TriggerData);
	
	ActivationBlockedTags.RemoveTag(AbilityTags::Status_Stun);
}

void UGAP_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (K2_HasAuthority())
	{
		AActor* Killer = TriggerEventData->ContextHandle.GetEffectCauser();
		if (!Killer || !AbilityTags::IsPlayer(Killer))
		{
			Killer = nullptr;
		}
		
		TArray<AActor*> RewardTargets = GetRewardTargets();
		if (RewardTargets.Num() == 0 && !Killer)
		{
			K2_EndAbility();
			return;
		}
		
		if (Killer && !RewardTargets.Contains(Killer))
		{
			RewardTargets.Add(Killer);
		}
		
		bool bFound = false;
		float SelfExp = GetAbilitySystemComponentFromActorInfo_Ensured()->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(), bFound);
		
		float TotalExpReward = BaseExpReward + ExpRewardPerExp * SelfExp;
		float TotalGoldReward = BaseGoldReward + GoldRewardPerExp * SelfExp;
		
		if (Killer)
		{
			float KillerExpReward = TotalExpReward * KillerRewardPortion;
			float KillerGoldReward = TotalGoldReward * KillerRewardPortion;
			
			FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
			EffectSpec.Data->SetSetByCallerMagnitude(AbilityTags::AttributeExperience, KillerExpReward);
			EffectSpec.Data->SetSetByCallerMagnitude(AbilityTags::AttributeGold, KillerGoldReward);
			
			K2_ApplyGameplayEffectSpecToTarget(EffectSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));
			
			TotalExpReward -= KillerExpReward;
			TotalGoldReward -= KillerGoldReward;
		}
		
		float ExperiencePerTarget = TotalExpReward / RewardTargets.Num();
		float GoldPerTarget = TotalGoldReward / RewardTargets.Num();
		
		FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
		EffectSpec.Data->SetSetByCallerMagnitude(AbilityTags::AttributeExperience, ExperiencePerTarget);
		EffectSpec.Data->SetSetByCallerMagnitude(AbilityTags::AttributeGold, GoldPerTarget);
		
		K2_ApplyGameplayEffectSpecToTarget(EffectSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(RewardTargets, true));
		K2_EndAbility();
	}
}

TArray<AActor*> UGAP_Dead::GetRewardTargets() const
{
	TSet<AActor*> OutActors;
	
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !GetWorld())
	{
		return OutActors.Array();
	}
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(RewardRange);
	
	TArray<FOverlapResult> OverlapResults;
	if (GetWorld()->OverlapMultiByObjectType(OverlapResults, AvatarActor->GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape))
	{
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(OverlapResult.GetActor());
			if (!OtherTeamInterface || OtherTeamInterface->GetTeamAttitudeTowards(*AvatarActor) != ETeamAttitude::Hostile)
			{
				continue;
			}
			
			if (!AbilityTags::IsPlayer(OverlapResult.GetActor()))
			{
				continue;
			}
			
			OutActors.Add(OverlapResult.GetActor());
		}
	}
	
	return OutActors.Array();
}
