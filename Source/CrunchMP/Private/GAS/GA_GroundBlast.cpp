// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_GroundBlast.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/AbilitySystemTags.h"
#include "GAS/TargetActor_GroundPick.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

UGA_GroundBlast::UGA_GroundBlast()
{
	ActivationOwnedTags.AddTag(AbilityTags::Status_Aim);
	BlockAbilitiesWithTag.AddTag(AbilityTags::BasicAttack);
}

void UGA_GroundBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
	{
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* PlayGroundBlastAnimTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, TargettingMontage);
	PlayGroundBlastAnimTask->OnBlendOut.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	PlayGroundBlastAnimTask->OnCancelled.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	PlayGroundBlastAnimTask->OnInterrupted.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	PlayGroundBlastAnimTask->OnCompleted.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	PlayGroundBlastAnimTask->ReadyForActivation();
	
	UAbilityTask_WaitTargetData* WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this, &UGA_GroundBlast::TargetConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this, &UGA_GroundBlast::TargetCancelled);
	WaitTargetDataTask->ReadyForActivation();
	
	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask->BeginSpawningActor(this, TargetActorClass, TargetActor);
	
	ATargetActor_GroundPick* GroundPickActor = Cast<ATargetActor_GroundPick>(TargetActor);
	if (GroundPickActor)
	{
		GroundPickActor->SetShouldShowDebug(ShouldShowDebug());
		GroundPickActor->SetTargetAreaRadius(TargetAreaRadius);
		GroundPickActor->SetTargetTraceRange(TargetTraceRange);
	}
	
	WaitTargetDataTask->FinishSpawningActor(this, TargetActor);
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	if (K2_HasAuthority())
	{
		BP_ApplyGameplayEffectToTarget(TargetDataHandle, DamageEffectDef.DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		PushTargets(TargetDataHandle, DamageEffectDef.PushVelocity);
	}
	
	FGameplayCueParameters BlastingCueParams;
	BlastingCueParams.Location = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, 1).ImpactPoint;
	BlastingCueParams.RawMagnitude = TargetAreaRadius;
	
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BlastGameplayCueTag, BlastingCueParams);
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(AbilityTags::GameplayCue_CameraShake, BlastingCueParams);
	
	UAnimInstance* OwnerAnimInst = GetOwnerAnimInst();
	if (OwnerAnimInst)
	{
		OwnerAnimInst->Montage_Play(CastMontage);
	}
	
	K2_EndAbility();
}

void UGA_GroundBlast::TargetCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	UE_LOG(LogTemp, Warning, TEXT("Target Cancelled"));
	K2_EndAbility();
}
