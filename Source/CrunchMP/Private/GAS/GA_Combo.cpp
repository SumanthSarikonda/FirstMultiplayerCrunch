// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "AbilitySystemTags.h"
#include "AbilitySystemComponent.h"
#include "GAS/BaseAttributeSet.h"

#include "GameplayTagsManager.h"
#include "AbilitySystemBlueprintLibrary.h"


UGA_Combo::UGA_Combo()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	SetAssetTags(FGameplayTagContainer(AbilityTags::BasicAttack));
	
	FGameplayTagContainer BlockTags;
	BlockTags.AddTag(AbilityTags::BasicAttack);
	
	BlockAbilitiesWithTag = BlockTags;
	
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo)) //ActivationInfo Is a Pointer So Referencing That Would Work
	{
		UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ComboMontage);
		PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->ReadyForActivation();
		
		UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboChangedEventTag(), nullptr, false, false );
		WaitComboChangeEvent->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangeEventReceived);
		WaitComboChangeEvent->ReadyForActivation();
	}
	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetingEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboTargetEventTag());
		WaitTargetingEventTask->EventReceived.AddDynamic(this, &UGA_Combo::DoDamage);
		WaitTargetingEventTask->ReadyForActivation();
	}
	
	NextComboName = NAME_None;
	SetupWaitInputPress();
	
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Change");
}

FGameplayTag UGA_Combo::GetComboChangedEventEndTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Change.End");
}

FGameplayTag UGA_Combo::GetComboTargetEventTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Damage");
}

void UGA_Combo::SetupWaitInputPress()
{
	UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this, &UGA_Combo::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void UGA_Combo::HandleInputPress(float TimeWaited)
{
	SetupWaitInputPress();
	TryCommitCombo();
}


void UGA_Combo::TryCommitCombo()
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
	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(ComboMontage), NextComboName, ComboMontage);
}

TSubclassOf<UGameplayEffect> UGA_Combo::GetDamageEffectForCurrentCombo() const
{
	UAnimInstance* OwnerAnimInst = GetOwnerAnimInst();
	if (!OwnerAnimInst)
	{
		return DefaultDamageEffect;
	}

	FName CurrentSectionName = OwnerAnimInst->Montage_GetCurrentSection(ComboMontage);

	// === SMART FIRST-HIT FIX (this is the one that finally kills the bug) ===
	if (CurrentSectionName.IsNone())
	{
		if (!DamageEffectMap.IsEmpty())
		{
			// Grab whatever key is first in your map (will always be combo01 on first hit)
			auto It = DamageEffectMap.CreateConstIterator();
			CurrentSectionName = It.Key();
		}
		else
		{
		}
	}

	if (const TSubclassOf<UGameplayEffect>* FoundEffectPtr = DamageEffectMap.Find(CurrentSectionName))
	{
		return *FoundEffectPtr;
	}

	// Final safety net
	return DefaultDamageEffect;
}

void UGA_Combo::ComboChangeEventReceived(FGameplayEventData Data)
{
	FGameplayTag EventTag = Data.EventTag;
	
	if (EventTag == GetComboChangedEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}
	
	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	
	NextComboName = TagNames.Last();
	
}

void UGA_Combo::DoDamage(FGameplayEventData Data)
{
	int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(Data.TargetData);
	
	for (int i = 0; i < HitResultCount; i++)
	{
		FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Data.TargetData, i);
		TSubclassOf<UGameplayEffect> GameplayEffect = GetDamageEffectForCurrentCombo();
		ApplyGameplayEffectToHitReultActor(HitResult, GameplayEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
	}
}


