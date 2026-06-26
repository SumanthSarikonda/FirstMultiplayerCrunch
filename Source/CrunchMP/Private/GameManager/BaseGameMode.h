// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "BaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
	virtual  void StartPlay() override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* Controller) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	
private:
	FGenericTeamId GetTeamIdForPlayer(const AController* InController) const;
	
	AActor* FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const;
	
	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TSubclassOf<APawn> BackUpPawn;
	
	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartTagMap;
	
	class AStormCore* GetStormCore() const;
	
	void MatchFinished(AActor* ViewTarget, int WinningTeam);
	
};
