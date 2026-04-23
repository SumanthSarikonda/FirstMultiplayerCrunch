// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_GroundPick.generated.h"

/**
 * 
 */
UCLASS()
class ATargetActor_GroundPick : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:
	ATargetActor_GroundPick();
	
	void SetTargetAreaRadius(float NewRadius);
	FORCEINLINE void SetTargetTraceRange(float NewRange) {TargetTraceRange = NewRange;} ;
	virtual void ConfirmTargetingAndContinue() override;
	void SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy = true);
	FORCEINLINE void SetShouldShowDebug(bool bShowDebug) {bShouldShowDebug = bShowDebug;} ;
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Visual")
	class UDecalComponent* DecalComp;
	
	bool bShouldTargetEnemy = true;
	bool bShouldTargetFriendly = false;
	
	virtual void Tick(float DeltaTime) override;
	
	FVector GetTargetPoint() const;
	
	UPROPERTY(EditAnywhere, Category = "Targetting")
	float TargetAreaRadius = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "Targetting")
	float TargetTraceRange = 2000.f;
	
	bool bShouldShowDebug = false;
};
