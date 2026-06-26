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

AActor* UBaseGameplayAbility::GetAimTarget(float AimDist, ETeamAttitude::Type TeamAttitude) const
{
	AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
	if (OwnerAvatarActor)
	{
		FVector Loc;
		FRotator Rot;
		OwnerAvatarActor->GetActorEyesViewPoint(Loc, Rot);
		
		FVector AimEnd = Loc + Rot.Vector() * AimDist;
		
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(OwnerAvatarActor);
		
		FCollisionObjectQueryParams CollisionObjectParams;
		CollisionObjectParams.AddObjectTypesToQuery(ECC_Pawn);
		
		if (ShouldShowDebug())
		{
			DrawDebugLine(GetWorld(), Loc, AimEnd, FColor::Red, false, 2.f, 0U, 3.f);
		}
		
		TArray<FHitResult> HitResults;
		if (GetWorld()->LineTraceMultiByObjectType(HitResults, Loc, AimEnd, CollisionObjectParams, CollisionParams))
		{
			for (FHitResult& HitResult : HitResults)
			{
				if (IsActorTeamAttitudeIs(HitResult.GetActor(), TeamAttitude))
				{
					return HitResult.GetActor();
				}
			}
		}
	}
	
	return nullptr;
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

void UBaseGameplayAbility::PushTargetsFromLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	const FVector& FromLoc, float PushSpeed)
{
	TArray<AActor*> Targets = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
	PushTargetsFromLocation(Targets, FromLoc, PushSpeed);
}

void UBaseGameplayAbility::PushTargetsFromLocation(const TArray<AActor*>& Targets, const FVector& FromLoc,
	float PushSpeed)
{
	for (AActor* Target : Targets)
	{
		FVector PushDir = Target->GetActorLocation() - FromLoc;
		PushDir.Z = 0;
		PushDir.Normalize();
		
		PushTarget(Target, PushDir * PushSpeed);
	}
}

void UBaseGameplayAbility::PlayMontageLocally(UAnimMontage* MontageToPlay)
{
	UAnimInstance* OwnerAnimIsnt = GetOwnerAnimInst();
	if (OwnerAnimIsnt && !OwnerAnimIsnt->Montage_IsPlaying(MontageToPlay))
	{
		OwnerAnimIsnt->Montage_Play(MontageToPlay);
	}
}

void UBaseGameplayAbility::StopMontageAfterCurrentSection(UAnimMontage* MontageToStop)
{
	UAnimInstance* OwnerAnimIsnt = GetOwnerAnimInst();
	if (OwnerAnimIsnt)
	{
		FName CurrenSectionName = OwnerAnimIsnt->Montage_GetCurrentSection(MontageToStop);
		OwnerAnimIsnt->Montage_SetNextSection(CurrenSectionName, NAME_None, MontageToStop);
	}
}

FGenericTeamId UBaseGameplayAbility::GetOwnerTeamId() const
{
	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	if (OwnerTeamInterface)
	{
		return OwnerTeamInterface->GetGenericTeamId();
	}
	
	return FGenericTeamId::NoTeam;
}

bool UBaseGameplayAbility::IsActorTeamAttitudeIs(const AActor* OtherActor, ETeamAttitude::Type TeamAttitude) const
{
	if (!OtherActor)
		return false;
	
	IGenericTeamAgentInterface* OwnerTeamAgentInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	if (OwnerTeamAgentInterface)
	{
		return OwnerTeamAgentInterface->GetTeamAttitudeTowards(*OtherActor) == TeamAttitude;
	}
	
	return false;
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

void UBaseGameplayAbility::SendLocalGameplayEvent(const FGameplayTag& Tag, const FGameplayEventData& EventData)
{
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (OwnerASC)
	{
		OwnerASC->HandleGameplayEvent(Tag, &EventData);
	}
}
