// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UpperCut.h"

#include "DebugHelper.h"
#include "GAS/GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayTagsManager.h"
#include "GAS/AbilitySystemTags.h"

UUpperCut::UUpperCut()
{
	BlockAbilitiesWithTag.AddTag(AbilityTags::BasicAttack);
}

void UUpperCut::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayUpperCutMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, UpperCutMontage);
		PlayUpperCutMontageTask->OnBlendOut.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCancelled.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnInterrupted.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCompleted.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->ReadyForActivation();
		
		UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetUpperCutLaunchTag());
		WaitLaunchEventTask->EventReceived.AddDynamic(this, &UUpperCut::StartLaunching);
		WaitLaunchEventTask->ReadyForActivation();
	}
	NextComboName = NAME_None;
}


FGameplayTag UUpperCut::GetUpperCutLaunchTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.UpperCut.Launch");
}

const FGenericDamageEffectDef* UUpperCut::GetDamageEffectDefForCurrentCombo() const
{
	UAnimInstance* OwnerAnimIsnt = GetOwnerAnimInst();
	if (OwnerAnimIsnt)
	{
		FName CurrentComboName = OwnerAnimIsnt->Montage_GetCurrentSection(UpperCutMontage);
		const FGenericDamageEffectDef* EffectDef = ComboDamageMap.Find(CurrentComboName);
		return EffectDef;
	}
	return nullptr;
}

void UUpperCut::StartLaunching(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutLaunchSpeed);
		int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(EventData.TargetData);
	
		for (int i = 0; i < HitResultCount; i++)
		{
			FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, i);
			PushTarget(HitResult.GetActor(), FVector::UpVector * UpperCutLaunchSpeed);
			ApplyGameplayEffectToHitReultActor(HitResult,LaunchDamageEffect,GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
	UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboChangedEventTag(), nullptr,false, false);
	WaitComboChangeEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboChangeEvent);
	WaitComboChangeEvent->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AbilityTags::BasicAttack_Pressed);
	WaitComboCommitEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboCommitEvent);
	WaitComboCommitEvent->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitComboDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboTargetEventTag());
	WaitComboDamageEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboDamageEvent);
	WaitComboDamageEvent->ReadyForActivation();
}

void UUpperCut::HandleComboChangeEvent(FGameplayEventData EventData)
{
	FGameplayTag EvenTag = EventData.EventTag;
	if (EvenTag == UGA_Combo::GetComboChangedEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}
	
	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EvenTag, TagNames);
	
	NextComboName = TagNames.Last();
}

void UUpperCut::HandleComboCommitEvent(FGameplayEventData EventData)
{
	
	if (NextComboName == NAME_None)
	{
		return;
	}
	
	UAnimInstance* OwnerAnimInst = GetOwnerAnimInst();
	if (!OwnerAnimInst)
	{
		return;
	}
	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(UpperCutMontage), NextComboName, UpperCutMontage);
}

void UUpperCut::HandleComboDamageEvent(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutComboStableSpeed);
		const FGenericDamageEffectDef* EffectDef = GetDamageEffectDefForCurrentCombo();
		if (!EffectDef)
		{
			return;
		}
		int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(EventData.TargetData);
	    for (int i = 0; i < HitResultCount; i++)
		{
			FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, i);
			FVector PushVelocity = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(EffectDef->PushVelocity);
			PushTarget(HitResult.GetActor(), PushVelocity);
			ApplyGameplayEffectToHitReultActor(HitResult,EffectDef->DamageEffect,GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}
