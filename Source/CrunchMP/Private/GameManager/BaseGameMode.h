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
	
private:
	FGenericTeamId GetTeamIdForPlayer(const APlayerController* PC) const;
	
	AActor* FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const;
	
	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartTagMap;
	
};
