// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/BaseGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "GAS/AbilitySystemTags.h"
#include "GAS/GAP_Launched.h"

class UAnimInstance* UBaseGameplayAbility::GetOwnerAnimInst() const
{
	USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
	if (OwnerSkeletalMeshComp)
	{
		return OwnerSkeletalMeshComp->GetAnimInstance();
	}
	return nullptr;
}

UBaseGameplayAbility::UBaseGameplayAbility()
{
	ActivationBlockedTags.AddTag(AbilityTags::Status_Stun);
}

bool UBaseGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (AbilitySpec && AbilitySpec->Level <= 0)
	{
		return false;
	}
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

TArray<FHitResult> UBaseGameplayAbility::GetHitResultsFromTargetData(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, ETeamAttitude::Type TargetTeam,
	bool bShowDebug, bool bIgnoreSelf) const
{
	TArray<FHitResult> OutResults;
	TSet<AActor*> HitActors;
	
	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	
	for (const TSharedPtr<FGameplayAbilityTargetData> TargetData : TargetDataHandle.Data)
	{
		FVector  StartLocation = TargetData->GetOrigin().GetTranslation();
		FVector EndLocation = TargetData->GetEndPoint();
		
		TArray<TEnumAsByte<EObjectTypeQuery> >  ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
		
		TArray<AActor*> ActorsToIgnore;
		if (bIgnoreSelf)
		{
			ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
		}
		
		EDrawDebugTrace::Type DrawDebugTrace = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		
		TArray<FHitResult> Results;
		UKismetSystemLibrary::SphereTraceMultiForObjects(this, StartLocation, EndLocation, SphereSweepRadius, ObjectTypes, false, ActorsToIgnore, DrawDebugTrace, Results, false);
		
		for (const FHitResult& Result : Results)
		{
			if (HitActors.Contains(Result.GetActor()))
			{
				continue;
			}
			
			if (OwnerTeamInterface)
			{
				ETeamAttitude::Type OtherActorTeamAttiude = OwnerTeamInterface->GetTeamAttitudeTowards(*Result.GetActor());
				if (OtherActorTeamAttiude !=TargetTeam)
				{
					continue;
				}
			}
			
			HitActors.Add(Result.GetActor());
			OutResults.Add(Result);
		}
	}
	return OutResults;
	
}

void UBaseGameplayAbility::PushSelf(const FVector& PushVelocity)
{
	ACharacter* OwningAvatarCharacter = GetOwningAvatarCharacter();
	if (OwningAvatarCharacter)
	{
		OwningAvatarCharacter->LaunchCharacter(PushVelocity, true, true);
	}
}

void UBaseGameplayAbility::PushTarget(AActor* Target, const FVector& PushVelocity)
{
	if (!Target)
		return;
	
	FGameplayEventData EventData;
	
	FGameplayAbilityTargetData_SingleTargetHit* HitData = new FGameplayAbilityTargetData_SingleTargetHit;
	FHitResult HitResult;
	HitResult.ImpactPoint = PushVelocity;
	HitData->HitResult = HitResult;
	EventData.TargetData.Add(HitData);
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, UGAP_Launched::GetLaunchedAbilityActivationTag(), EventData);
}

void UBaseGameplayAbility::PushTargets(const TArray<AActor*>& Targets, const FVector& PushVelocity)
{
	for (AActor* Target : Targets)
	{
		PushTarget(Target, PushVelocity);
	}
}

void UBaseGameplayAbility::PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	const FVector& PushVelocity)
{
	TArray<AActor*> Targets = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
	PushTargets(Targets, PushVelocity);
}

ACharacter* UBaseGameplayAbility::GetOwningAvatarCharacter()
{
	if (!AvatarCharacter)
	{
		AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	}
	return AvatarCharacter;
}

void UBaseGameplayAbility::ApplyGameplayEffectToHitReultActor(const FHitResult& HitResult,
	TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	FGameplayEffectSpecHandle EffectSpechandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, Level);
	
	FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	EffectContext.AddHitResult(HitResult);
	
	EffectSpechandle.Data->SetContext(EffectContext);
	
	ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), EffectSpechandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
}
