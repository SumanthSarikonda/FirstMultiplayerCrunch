// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/AN_SendTargetGroup.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameplayEffectTypes.h"

void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp)
		return;
	
	if (TargetSocketNames.Num() <= 1)
		return;
	
	if (!MeshComp->GetOwner() || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
	{
		return;
	}
	
	FGameplayEventData Data;
	TSet<AActor*> HitActors;
	AActor* OwnerActor = MeshComp->GetOwner();
	const IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwnerActor);
	
	for (int i = 1; i < TargetSocketNames.Num(); ++i)
	{
		FVector StartLocation = MeshComp->GetSocketLocation(TargetSocketNames[i-1]);
		FVector EndLocation = MeshComp->GetSocketLocation(TargetSocketNames[i]);
		
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
		TArray<FHitResult> HitResults;
		
		TArray<AActor*> ActorsToIgnore;
		if (bIgnoreOwner)
		{
			ActorsToIgnore.Add(OwnerActor);
		}
		
		EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		
		UKismetSystemLibrary::SphereTraceMultiForObjects(
			 MeshComp,
			 StartLocation,
			 EndLocation,
			 SphereSweepRadius,
			 ObjectTypes,
			 false,
			 ActorsToIgnore,
			 DrawDebugTrace,
			 HitResults,
			 false
			 );
		
		for (const FHitResult& HitResult : HitResults)
		{
			if (HitActors.Contains(HitResult.GetActor()))
			{
				continue;
			}
			
			if (OwnerTeamInterface)
			{
				if (OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult.GetActor()) != TargetTeam)
				{
					continue;
				}
			}
			
			FGameplayAbilityTargetData_SingleTargetHit* TargetHit = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
			Data.TargetData.Add(TargetHit);
			SendLocalGameplayCue(HitResult);
		}		
	}
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, Data);
}

void UAN_SendTargetGroup::SendLocalGameplayCue(const FHitResult& HitResult) const
{
	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal;
	
	for (const FGameplayTag& GameplayCueTag : TriggerGameplayCueTags)
	{
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(HitResult.GetActor(), GameplayCueTag, EGameplayCueEvent::Executed, CueParams);
		
	}
}
