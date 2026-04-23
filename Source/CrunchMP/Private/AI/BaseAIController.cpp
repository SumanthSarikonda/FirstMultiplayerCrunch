// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BaseAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Character/BaseCharacter.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/AbilitySystemTags.h"

#include "DebugHelper.h"

ABaseAIController::ABaseAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerceptionComponent");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");
	
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	
	SightConfig->SightRadius = 1000.f;
	SightConfig->LoseSightRadius = 1200.f;
	
	SightConfig->SetMaxAge(5.f);
	
	SightConfig->PeripheralVisionAngleDegrees = 180.f;
	
	AIPerceptionComponent->ConfigureSense(*SightConfig);
	
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::TargetPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ABaseAIController::TargetForgotten);
	
}

void ABaseAIController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	
	IGenericTeamAgentInterface* PawnTeamInterface  = Cast<IGenericTeamAgentInterface>(NewPawn);
	if (PawnTeamInterface)
	{
		SetGenericTeamId(PawnTeamInterface->GetGenericTeamId());
		ClearAndDisableAllSenses();
		EnableAllSenses();
	}
	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NewPawn);
	if (PawnASC)
	{
		PawnASC->RegisterGameplayTagEvent(AbilityTags::Status_Dead).AddUObject(this, &ABaseAIController::PawnDeadTagUpdated);
		PawnASC->RegisterGameplayTagEvent(AbilityTags::Status_Stun).AddUObject(this, &ABaseAIController::PawnStunTagUpdated);
	}
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(BehaviorTree);
}

void ABaseAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		if (!GetCurrentTarget())
		{
			Debug::print(TargetActor->GetName() + ": No target");
			SetCurrentTarget(TargetActor);
		}
	}
	else
	{
		ForgetActorIfDead(TargetActor);
	}
}

void ABaseAIController::TargetForgotten(AActor* ForgottenActor)
{
	if (!ForgottenActor)
		return;
	
	if (GetCurrentTarget() == ForgottenActor)
	{
		SetCurrentTarget(GetNextPerceivedActor());
	}
}

const UObject* ABaseAIController::GetCurrentTarget() const
{
	const UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (BlackboardComponent)
	{
		return  GetBlackboardComponent()->GetValueAsObject(TargetBBKeyName);
	}
	return nullptr;
}

void ABaseAIController::SetCurrentTarget(AActor* NewTarget)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent)
		return;
	
	if (NewTarget)
	{
		BlackboardComponent->SetValueAsObject(TargetBBKeyName, NewTarget);
	}
	else
	{
		BlackboardComponent->ClearValue(TargetBBKeyName);
	}
}

AActor* ABaseAIController::GetNextPerceivedActor() const
{
	if (AIPerceptionComponent)
	{
		TArray<AActor*> PerceivedActors;
		AIPerceptionComponent->GetPerceivedHostileActors(PerceivedActors);
		
		if (PerceivedActors.Num() != 0)
		{
			return PerceivedActors[0];
		}
	}
	return nullptr;
}

void ABaseAIController::ForgetActorIfDead(AActor* ActorToForget)
{
	const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToForget);
	if (!ActorASC)
		return;
	
	if (ActorASC->HasMatchingGameplayTag(AbilityTags::Status_Dead))
	{
		for (UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iter = AIPerceptionComponent->GetPerceptualDataIterator(); Iter; ++Iter)
		{
			if (Iter->Key != ActorToForget)
			{
				continue;
			}
			for (FAIStimulus& Stimuli : Iter->Value.LastSensedStimuli)
			{
				Stimuli.SetStimulusAge(TNumericLimits<float>::Max());
			}
		}
	}
	
}

void ABaseAIController::ClearAndDisableAllSenses()
{
	AIPerceptionComponent->AgeStimuli(TNumericLimits<float>::Max());
	
	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), false);
	}
	
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->ClearValue(TargetBBKeyName);
	}
}

void ABaseAIController::EnableAllSenses()
{
	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), true);
	}
}

void ABaseAIController::PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (Count != 0)
	{
		GetBrainComponent()->StopLogic("Dead");
		ClearAndDisableAllSenses();
		bIspawnDead = true;
	}
	else
	{
		GetBrainComponent()->StartLogic();
		EnableAllSenses();
		bIspawnDead = false;
	}
}

void ABaseAIController::PawnStunTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (bIspawnDead)
		return;
	
	if (Count != 0)
	{
		GetBrainComponent()->StopLogic("Stun");
	}
	else
	{
		GetBrainComponent()->StartLogic();
	}
}
