// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MinnionSpawner.h"

#include "Minion.h"
#include "GameFramework/PlayerStart.h"
#include "AI/Minion.h"
#include "Materials/MaterialExpressionOperator.h"

// Sets default values
AMinnionSpawner::AMinnionSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMinnionSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(SpawnNewIntervalTimerHandle, this, &AMinnionSpawner::SpawnNewGroup, GroupSpawnInterval, true);
	}
}

// Called every frame
void AMinnionSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

const APlayerStart* AMinnionSpawner::GetNextSpawnSpot()
{
	if (SpawnSpots.Num() == 0)
	{
		return nullptr;
	}
	++NextSpawnSpotIndex;
	
	if (NextSpawnSpotIndex >= SpawnSpots.Num())
	{
		NextSpawnSpotIndex = 0;
	}
	return SpawnSpots[NextSpawnSpotIndex];
}

void AMinnionSpawner::SpawnNewGroup()
{
	int i = MinionPerGroup;
	
	while (i > 0)
	{
		FTransform SpawnTransform = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnSpot())
		{
			SpawnTransform = NextSpawnSpot->GetActorTransform();
		}
		AMinion* NextAvailableMinion = GetNextAvailableMinion();
		if (!NextAvailableMinion)
			break;
		
		NextAvailableMinion->SetActorTransform(SpawnTransform);
		NextAvailableMinion->Activate();
		--i;
	}
	SpawnNewMinions(i);
}

void AMinnionSpawner::SpawnNewMinions(int Amount)
{
	for (int i = 0; i < Amount; i++)
	{
		FTransform SpawnTransform = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnSpot())
		{
			SpawnTransform = NextSpawnSpot->GetActorTransform();
		}
		AMinion* NewMinion = GetWorld()->SpawnActorDeferred<AMinion>(MinionClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		NewMinion->SetGenericTeamId(SpawnerTeamID);
		NewMinion->FinishSpawning(SpawnTransform);
		NewMinion->SetGoal(Goal);
		MinionPool.Add(NewMinion);
	}
}

AMinion* AMinnionSpawner::GetNextAvailableMinion() const
{
	for (AMinion * Minion : MinionPool)
	{
		if (!Minion->IsActive())
		{
			return Minion;
		}
	}
	return nullptr;
}

