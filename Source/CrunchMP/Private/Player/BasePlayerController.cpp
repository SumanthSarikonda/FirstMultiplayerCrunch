
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystemInterface.h"
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

void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (Subsystem)
		{
			Subsystem->RemoveMappingContext(UIInputMapping);
			Subsystem->AddMappingContext(UIInputMapping, 1);
		}
	
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (EIC)
	{
		EIC->BindAction(IA_ToggleShop, ETriggerEvent::Triggered, this, &ABasePlayerController::ToggleShop);
		EIC->BindAction(IA_GameplayMenu, ETriggerEvent::Triggered, this, &ABasePlayerController::ToggleGameplayMenu);
	}
}

void ABasePlayerController::MatchFinished(AActor* ViewTarget, int WinningTeam)
{
	if (!HasAuthority())
		return;
	
	BasePlayerCharacter->DisableInput(this);
	Client_MatchFinished(ViewTarget, WinningTeam);
}

void ABasePlayerController::Client_MatchFinished_Implementation(AActor* ViewTarget, int WinningTeam)
{
	SetViewTargetWithBlend(ViewTarget, MatchFinishedViewBlendTimeDuration);
	FString WinLoseMsg = "You Win";
	if (GetGenericTeamId().GetId() != WinningTeam)
	{
		WinLoseMsg = "You Lose";
	}
	
	GameplayWidget->SetGameplayMenuTitle(WinLoseMsg);
	FTimerHandle ShowWinLoseTimerHandle;
	GetWorldTimerManager().SetTimer(ShowWinLoseTimerHandle, this, &ABasePlayerController::ShowWinLoseState, MatchFinishedViewBlendTimeDuration);
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

void ABasePlayerController::ToggleShop()
{
	if (GameplayWidget)
	{
		GameplayWidget->ToggleShop();
	}
}

void ABasePlayerController::ToggleGameplayMenu()
{
	if (GameplayWidget)
	{
		GameplayWidget->ToggleGameplayMenu();
	}
}

void ABasePlayerController::ShowWinLoseState()
{
	if(GameplayWidget)
	{
		GameplayWidget->ShowGameplayMenu();
	}
}
