// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/TargetActor_GroundPick.h"
#include "Components/DecalComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/OverlapResult.h"
#include "CrunchMP/CrunchMP.h"
#include  "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"

ATargetActor_GroundPick::ATargetActor_GroundPick()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Comp"));
	
	DecalComp = CreateDefaultSubobject<UDecalComponent>("Decal Comp");
	DecalComp->SetupAttachment(GetRootComponent());
}

void ATargetActor_GroundPick::SetTargetAreaRadius(float NewRadius)
{
	TargetAreaRadius = NewRadius;
	DecalComp->DecalSize = FVector{NewRadius};
}

void ATargetActor_GroundPick::ConfirmTargetingAndContinue()
{
	TArray<FOverlapResult> OverlapResults;
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(TargetAreaRadius);
	
	GetWorld()->OverlapMultiByObjectType(OverlapResults,
		GetActorLocation(),
		FQuat::Identity, 
		ObjectQueryParams,
		CollisionShape
		);
	
	TSet<AActor*> TargetActors;
	
	IGenericTeamAgentInterface* OwnerTeamInterface = nullptr;
	if (OwningAbility)
	{
		OwnerTeamInterface  = Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());
	}
	
	for(const FOverlapResult& OverlapResult : OverlapResults)
	{
		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Friendly && !bShouldTargetFriendly)
			continue;
		
		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Hostile && !bShouldTargetEnemy)
			continue;
		
		TargetActors.Add(OverlapResult.GetActor());
	}
	
	FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors.Array(), false);
	
	FGameplayAbilityTargetData_SingleTargetHit* HitLoc = new FGameplayAbilityTargetData_SingleTargetHit;
	HitLoc->HitResult.ImpactPoint = GetActorLocation();
	
	TargetData.Add(HitLoc);
	
	TargetDataReadyDelegate.Broadcast(TargetData);
}

void ATargetActor_GroundPick::SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy)
{
	bShouldTargetFriendly = bTargetFriendly;
	bShouldTargetEnemy = bTargetEnemy;
}

void ATargetActor_GroundPick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
	{
		SetActorLocation(GetTargetPoint());
	}
}

FVector ATargetActor_GroundPick::GetTargetPoint() const
{
	if (!PrimaryPC || !PrimaryPC->IsLocalPlayerController())
		return GetActorLocation();
	
	FHitResult TraceResult;
	
	FVector LookLoc;
	FRotator LookRot;
	
	PrimaryPC->GetPlayerViewPoint(LookLoc, LookRot);
	
	FVector TraceEnd = LookLoc + LookRot.Vector() * TargetTraceRange;
	
	GetWorld()->LineTraceSingleByChannel(TraceResult, LookLoc, TraceEnd, ECC_Target);
	
	if (!TraceResult.bBlockingHit)
	{
		GetWorld()->LineTraceSingleByChannel(TraceResult, TraceEnd, TraceEnd + FVector::DownVector * TNumericLimits<float>::Max(), ECC_Target);
	}
	
	if (!TraceResult.bBlockingHit)
	{
		return GetActorLocation();
	}
	
	if (bShouldShowDebug)
	{
		DrawDebugSphere(GetWorld(), TraceResult.ImpactPoint, TargetAreaRadius, 32, FColor::Red);
	}
	
	return TraceResult.ImpactPoint;
}
