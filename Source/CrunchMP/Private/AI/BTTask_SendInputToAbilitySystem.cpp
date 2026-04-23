// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_SendInputToAbilitySystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

EBTNodeResult::Type UBTTask_SendInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* OwnerAIC = OwnerComp.GetAIOwner();
	if (OwnerAIC)
	{
		UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIC->GetPawn());
		if (OwnerASC)
		{
			OwnerASC->PressInputID((int32)InputID);
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
