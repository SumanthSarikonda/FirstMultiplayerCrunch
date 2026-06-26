// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"
#include "CrunchMP/CrunchMP.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/BaseAbilitySystemComponent.h"
#include "GAS/BaseAttributeSet.h"
#include "GAS/AbilitySystemTags.h"
#include "Widgets/OverHeadStatsGauge.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

#include "DebugHelper.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_SpringArm, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Target, ECR_Ignore);
	
	BaseAbilitySystemComponent = CreateDefaultSubobject<UBaseAbilitySystemComponent>("BaseAbilitySystemComponent");
	BaseAbilitySystemComponent->SetIsReplicated(true);
	BaseAttributeSet = CreateDefaultSubobject<UBaseAttributeSet>("BaseAttributeSet");
	
	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());
	
	BindGASChangeDelegates();
	
	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("PerceptionStimuliSourceComponent");
}

void ABaseCharacter::ServerSideInit()
{
	BaseAbilitySystemComponent->InitAbilityActorInfo(this, this);
	BaseAbilitySystemComponent->ServerSideInit();
}

void ABaseCharacter::ClientSideInit()
{
	BaseAbilitySystemComponent->InitAbilityActorInfo(this, this);
}

bool ABaseCharacter::bIsControlledByLocalPlayer() const
{
	return GetController() != nullptr && GetController()->IsLocalPlayerController();
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	
	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
}

const TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>>& ABaseCharacter::GetAbilities() const
{
	return BaseAbilitySystemComponent->GetAbilities();
}

FVector ABaseCharacter::GetCaptureLocalPos() const
{
	return HeadShotCaptureLocalPos;
}

FRotator ABaseCharacter::GetCaptureLocalRot() const
{
	return HeadShotCaptureLocalRot;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	ConfigureOverHeadStatusWidget();
	MeshRelativeTransform = GetMesh()->GetRelativeTransform();
	
	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return BaseAbilitySystemComponent;
}

void ABaseCharacter::Server_SendGameplayEventToSelf_Implementation(const FGameplayTag& EventTag,
	const FGameplayEventData& EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, EventData);
}

bool ABaseCharacter::Server_SendGameplayEventToSelf_Validate(const FGameplayTag& EventTag,
	const FGameplayEventData& EventData)
{
	return true;
}

void ABaseCharacter::UpgradeAbilityWithInputId(EAbilityInputID InputID)
{
	if (BaseAbilitySystemComponent)
	{
		BaseAbilitySystemComponent->Server_UpgradeAbilityWithInputId(InputID);
	}
}

void ABaseCharacter::BindGASChangeDelegates()
{
	
	if (BaseAbilitySystemComponent)
	{
		BaseAbilitySystemComponent->RegisterGameplayTagEvent(AbilityTags::Status_Dead, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ABaseCharacter::DeathTagUpdated);
		BaseAbilitySystemComponent->RegisterGameplayTagEvent(AbilityTags::Status_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ABaseCharacter::StunTagUpdated);
		BaseAbilitySystemComponent->RegisterGameplayTagEvent(AbilityTags::Status_Aim, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ABaseCharacter::AimTagUpdated);
		BaseAbilitySystemComponent->RegisterGameplayTagEvent(AbilityTags::Status_Focus, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ABaseCharacter::FocusTagUpdated);
		
		BaseAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &ABaseCharacter::MoveSpeedUpdated);
		BaseAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ABaseCharacter::MaxHealthUpdated);
		BaseAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetMaxManaAttribute()).AddUObject(this, &ABaseCharacter::MaxManaUpdated);
	}
}

void ABaseCharacter::DeathTagUpdated(const FGameplayTag, int32 NewCount)
{
	UE_LOG(LogTemp, Warning, TEXT("Death Tag called"));

	if (NewCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Death] Starting death sequence"));
		StartDeathSequence();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Death] Respawning"));
		Respawn();
	}
}

void ABaseCharacter::StunTagUpdated(const FGameplayTag, int32 NewCount)
{
	if (IsDead()) return;
	
	if (NewCount != 0)
	{
		OnStun();
		PlayAnimMontage(StunMontage);
	}
	else
	{
		OnRecoverFromStun();
		StopAnimMontage(StunMontage);
	}
}

void ABaseCharacter::AimTagUpdated(const FGameplayTag, int32 NewCount)
{
	SetIsAiming(NewCount != 0);
}

void ABaseCharacter::FocusTagUpdated(const FGameplayTag, int32 NewCount)
{
	bIsInFocusMode = NewCount > 0;
}

void ABaseCharacter::SetIsAiming(bool bIsAiming)
{
	bUseControllerRotationYaw = bIsAiming;
	GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;
	OnAimStateChanged(bIsAiming);
}

void ABaseCharacter::OnAimStateChanged(bool bIsAiming)
{
	//Override in Child Class
}

void ABaseCharacter::MoveSpeedUpdated(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void ABaseCharacter::MaxHealthUpdated(const FOnAttributeChangeData& Data)
{
	if (IsValid(BaseAttributeSet))
	{
		BaseAttributeSet->ReScaleHealth();
	}
}

void ABaseCharacter::MaxManaUpdated(const FOnAttributeChangeData& Data)
{
	if (IsValid(BaseAttributeSet))
	{
		BaseAttributeSet->ReScaleMana();
	}
}

void ABaseCharacter::ConfigureOverHeadStatusWidget()
{
	if (!OverHeadWidgetComponent)
	{
		return;
	}
	
	if (bIsControlledByLocalPlayer())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		return;
	}

	UOverHeadStatsGauge* OverHeadStatsGauge = Cast<UOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	if (OverHeadStatsGauge)
	{
		OverHeadStatsGauge->ConfigureWithASC(GetAbilitySystemComponent());
		OverHeadWidgetComponent->SetHiddenInGame(false);
		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityTimerHandle);
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityTimerHandle, this, &ABaseCharacter::UpdateHeadStatGaugeVisibility, HeadStatGaugeVisibilityCheckUpdateGap, true);
	}
}

void ABaseCharacter::UpdateHeadStatGaugeVisibility()
{
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LocalPlayerPawn)
	{
		float DistSquared = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared > HeadStatGaugeVisibilityRangeSquared);
	}
}

void ABaseCharacter::SetStatusGuageEnabled(bool bIsEnable)
{
	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityTimerHandle);
	if (bIsEnable)
	{
		ConfigureOverHeadStatusWidget();
	}
	else
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
	}
}

void ABaseCharacter::OnStun()
{
}

void ABaseCharacter::OnRecoverFromStun()
{
}

bool ABaseCharacter::IsDead() const
{
	return  GetAbilitySystemComponent()->HasMatchingGameplayTag(AbilityTags::Status_Dead);
}

void ABaseCharacter::RespawnImmediately()
{
	if (HasAuthority())
		GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(AbilityTags::Status_Dead));
}

void ABaseCharacter::DeathMontageFinished()
{
	if (IsDead())
		SetRagdollEnabled(true);
}

void ABaseCharacter::SetRagdollEnabled(bool bIsEnable)
{
	if (bIsEnable)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

void ABaseCharacter::PlayDeathAnim()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &ABaseCharacter::DeathMontageFinished, MontageDuration + DeathMontageTimeShift);
	}
}

void ABaseCharacter::StartDeathSequence()
{
	OnDead();
	
	if (BaseAbilitySystemComponent)
	{
		BaseAbilitySystemComponent->CancelAllAbilities();
	}
	
	PlayDeathAnim();
	SetStatusGuageEnabled(false);
	
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetAiPerceptionStimuliSourceEnabled(false);
}

void ABaseCharacter::Respawn()
{
	OnRespawn();
	SetAiPerceptionStimuliSourceEnabled(true);
	SetRagdollEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGuageEnabled(true);
	
	if (HasAuthority() && GetController())
	{
		TWeakObjectPtr<AActor>  StartSpot = GetController()->StartSpot;
		if (StartSpot.IsValid())
		{
			SetActorTransform(StartSpot->GetActorTransform());
		}
	}
	
	if (BaseAbilitySystemComponent)
	{
		BaseAbilitySystemComponent->ApplyAllStats();
	}
}

void ABaseCharacter::OnDead()
{
}

void ABaseCharacter::OnRespawn()
{
}

void ABaseCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamId = NewTeamID;
}

FGenericTeamId ABaseCharacter::GetGenericTeamId() const
{
	return TeamId;
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseCharacter, TeamId);
}

void ABaseCharacter::OnRep_TeamID()
{
	//override in child class.
}

void ABaseCharacter::SetAiPerceptionStimuliSourceEnabled(bool bIsEnabled)
{
	if (!PerceptionStimuliSourceComponent)
	{
		return;
	}
	if (bIsEnabled)
	{
		PerceptionStimuliSourceComponent->RegisterComponent();
	}
	else
	{
		PerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}

