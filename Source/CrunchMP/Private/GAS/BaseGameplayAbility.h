// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "BaseGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class UBaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBaseGameplayAbility();
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
protected:
	AActor* GetAimTarget(float AimDist, ETeamAttitude::Type TeamAttitude) const;
	class UAnimInstance* GetOwnerAnimInst() const;
	
	TArray<FHitResult> GetHitResultsFromTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius = 30.f, ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile, bool bShowDebug = false, bool bIgnoreSelf = true) const;
	
	UFUNCTION()
	FORCEINLINE bool ShouldShowDebug() const {return bShouldShowDebug;}
	void PushSelf(const FVector& PushVelocity);
	void PushTarget(AActor* Target, const FVector& PushVelocity);
	void PushTargets(const TArray<AActor*>& Targets, const FVector& PushVelocity);
	void PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle,const FVector& PushVelocity);
	void PushTargetsFromLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle,const FVector& FromLoc, float PushSpeed);
	void PushTargetsFromLocation(const TArray<AActor*>& Targets, const FVector& FromLoc, float PushSpeed);
	void PlayMontageLocally(UAnimMontage* MontageToPlay);
	void StopMontageAfterCurrentSection(UAnimMontage* MontageToStop);
	FGenericTeamId GetOwnerTeamId() const;
	
	bool IsActorTeamAttitudeIs(const AActor* OtherActor, ETeamAttitude::Type TeamAttitude) const; 
	
	ACharacter* GetOwningAvatarCharacter();
	void ApplyGameplayEffectToHitReultActor(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	void SendLocalGameplayEvent(const FGameplayTag& Tag, const FGameplayEventData& EventData);
private:
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShouldShowDebug = false;
	
	UPROPERTY()
	class ACharacter* AvatarCharacter;
};
