// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "BasePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ABasePlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public: 
	void OnPossess(APawn* NewPawn) override; // Only Called On Server
	
	void AcknowledgePossession(APawn* NewPawn); // Only Called On Client, also on Listening Server
	
	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
private:
	void SpawnGameplayWidget();
	
	UPROPERTY()
	class ABasePlayerCharacter* BasePlayerCharacter;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UGameplayWidget> GameplayWidgetClass;
	
	UPROPERTY()
	class UGameplayWidget* GameplayWidget;
	
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;
};
