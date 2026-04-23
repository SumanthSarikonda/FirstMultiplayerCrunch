#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CGameplayAbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None                    UMETA(DisplayName = "None"),
	BasicAttack             UMETA(DisplayName = "Basic Attack"),
	Ability1                UMETA(DisplayName = "Ability One"),
	Ability2                UMETA(DisplayName = "Ability Two"),
	Ability3                UMETA(DisplayName = "Ability Three"),
	Ability4                UMETA(DisplayName = "Ability Four"),
	Ability5                UMETA(DisplayName = "Ability Five"),
	Ability6                UMETA(DisplayName = "Ability Six"),
	Confirm                 UMETA(DisplayName = "Confirm"),
	Cancel                  UMETA(DisplayName = "Cancel")
};

USTRUCT(BlueprintType)
struct FGenericDamageEffectDef
{
	GENERATED_BODY()
	
public:
	FGenericDamageEffectDef();
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	UPROPERTY(EditAnywhere)
	FVector PushVelocity;
};

USTRUCT(BlueprintType)
struct FHeroBaseStats : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	FHeroBaseStats();
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;
	
	UPROPERTY(EditAnywhere)
	float Strength;
	
	UPROPERTY(EditAnywhere)
	float Intelligence;
	
	UPROPERTY(EditAnywhere)
	float StrengthGrowthRate;
	
	UPROPERTY(EditAnywhere)
	float IntelligenceGrowthRate;
	
	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;
	
	UPROPERTY(EditAnywhere)
	float BaseMaxMana;
	
	UPROPERTY(EditAnywhere)
	float BaseAttackDamage;
	
	UPROPERTY(EditAnywhere)
	float BaseArmor;
	
	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;
};