// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "BaseAIController.generated.h"

struct FAIStimulus;
/**
 * 
 */
UCLASS()
class ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABaseAIController();
	
	virtual void OnPossess(APawn* NewPawn) override;
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetBBKeyName = "Target";
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;
	
	UPROPERTY(VisibleAnywhere, Category = "Perception")
	class UAIPerceptionComponent* AIPerceptionComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Perception")
	class UAISenseConfig_Sight* SightConfig;
	
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);
	
	UFUNCTION()
	void TargetForgotten(AActor* ForgottenActor);
	
	const UObject* GetCurrentTarget() const;
	void SetCurrentTarget(AActor* NewTarget);
	
	AActor* GetNextPerceivedActor() const;
	
	void ForgetActorIfDead(AActor* ActorToForget);
	
	void ClearAndDisableAllSenses();
	void EnableAllSenses();
	
	void PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count);
	void PawnStunTagUpdated(const FGameplayTag Tag, int32 Count);
	
	bool bIspawnDead = false;
};
