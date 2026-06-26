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
	virtual void SetupInputComponent() override;
	void MatchFinished(AActor* ViewTarget, int WinningTeam);
	
private:
	UFUNCTION(Client, Reliable)
	void Client_MatchFinished(AActor* ViewTarget, int WinningTeam);
	
	void SpawnGameplayWidget();
	
	UPROPERTY(EditDefaultsOnly, Category = "View")
	float MatchFinishedViewBlendTimeDuration = 2.f;
	
	UPROPERTY()
	class ABasePlayerCharacter* BasePlayerCharacter;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UGameplayWidget> GameplayWidgetClass;
	
	UPROPERTY()
	class UGameplayWidget* GameplayWidget;
	
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* UIInputMapping;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_ToggleShop;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_GameplayMenu;
	
	UFUNCTION()
	void ToggleShop();
	
	UFUNCTION()
	void ToggleGameplayMenu();
	
	void ShowWinLoseState();
};
