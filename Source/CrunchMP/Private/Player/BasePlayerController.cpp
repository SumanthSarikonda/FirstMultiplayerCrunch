
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/GameplayWidget.h"

void ABasePlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	BasePlayerCharacter = Cast<ABasePlayerCharacter>(NewPawn);
	if (BasePlayerCharacter)
	{
		BasePlayerCharacter->ServerSideInit();
		BasePlayerCharacter->SetGenericTeamId(TeamId);
	}
}

void ABasePlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	BasePlayerCharacter = Cast<ABasePlayerCharacter>(NewPawn);
	if (BasePlayerCharacter)
	{
		BasePlayerCharacter->ClientSideInit();
		SpawnGameplayWidget();
	}
}

void ABasePlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamId = NewTeamID;
}

FGenericTeamId ABasePlayerController::GetGenericTeamId() const
{
	return TeamId;
}

void ABasePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABasePlayerController, TeamId);
}

void ABasePlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController())
		return;
	
	GameplayWidget = CreateWidget<UGameplayWidget>(this, GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
		GameplayWidget->ConfigureAbilities(BasePlayerCharacter->GetAbilities());
	}
}
