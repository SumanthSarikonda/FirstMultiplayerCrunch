// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/BaseGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

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

FGenericTeamId ABaseGameMode::GetTeamIdForPlayer(const APlayerController* PC) const
{
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
