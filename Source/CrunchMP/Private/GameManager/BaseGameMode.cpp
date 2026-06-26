// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/BaseGameMode.h"
#include "Player/BasePlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Controller.h"
#include "Player/BasePlayerState.h"
#include "GameManager/StormCore.h"
#include "EngineUtils.h"

APlayerController* ABaseGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPC = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPC);
	
	FGenericTeamId TeamId = GetTeamIdForPlayer(NewPC);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}
	
	NewPC->StartSpot = FindNextStartSpotForTeam(TeamId);
	return NewPC;
}

void ABaseGameMode::StartPlay()
{
	Super::StartPlay();
	AStormCore* StormCore = GetStormCore();
	if (StormCore)
	{
		StormCore->OnGoalReached.AddUObject(this, &ABaseGameMode::MatchFinished);
	}
}

UClass* ABaseGameMode::GetDefaultPawnClassForController_Implementation(AController* Controller)
{
	ABasePlayerState* BasePlayerState = Controller->GetPlayerState<ABasePlayerState>();
	if (BasePlayerState && BasePlayerState->GetSelectedPawnClass())
	{
		return BasePlayerState->GetSelectedPawnClass();
	}
	
	return BackUpPawn;
}

APawn* ABaseGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayer);
	FGenericTeamId TeamId = GetTeamIdForPlayer(NewPlayer);
	
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}
	
	StartSpot = FindNextStartSpotForTeam(TeamId);
	NewPlayer->StartSpot = StartSpot;
	
	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

FGenericTeamId ABaseGameMode::GetTeamIdForPlayer(const AController* InController) const
{
	ABasePlayerState* BasePlayerState = InController->GetPlayerState<ABasePlayerState>();
	if (BasePlayerState && BasePlayerState->GetSelectedPawnClass())
	{
		return BasePlayerState->GetTeamIdBasedOnSlot();
	}
	
	static int PlayerCount = 0;
	++PlayerCount;
	return FGenericTeamId(PlayerCount % 2);
}

AActor* ABaseGameMode::FindNextStartSpotForTeam(const FGenericTeamId& TeamID) const
{
	const FName* StartSpotTag = TeamStartTagMap.Find(TeamID);
	if (!StartSpotTag)
	{
		return nullptr;
	}
	
	UWorld* World = GetWorld();
	
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}
	return nullptr;
}

void ABaseGameMode::MatchFinished(AActor* ViewTarget, int WinningTeam)
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ABasePlayerController> It(World); It; ++It)
		{
			It->MatchFinished(ViewTarget, WinningTeam);
		}
	}
}

class AStormCore* ABaseGameMode::GetStormCore() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<AStormCore> It(World); It; ++It)
		{
			return *It;
		}
	}
	
	return nullptr;
}
