// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "InventoryItem.generated.h"

class UPA_ShopItem;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityCanCastUpdatedDelegate, bool /* bCanCast */);

USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()
public:
	FInventoryItemHandle();
	static FInventoryItemHandle InvalidHandle();
	static FInventoryItemHandle CreateHandle();
	
	bool IsValid() const;
	uint32 GetHandleId() const {return HandleId;}
	
private:
	explicit FInventoryItemHandle(uint32 Id);
	
	UPROPERTY()
	uint32 HandleId;
	
	static uint32 GenerateNextID();
	static uint32 GetInvalidId();
};

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs);
uint32 GetTypeHash(const FInventoryItemHandle& Key);

/**
 * 
 */
UCLASS()
class UInventoryItem : public UObject
{
	GENERATED_BODY()
	
public:
	FOnAbilityCanCastUpdatedDelegate OnAbilityCanCastUpdated;
	
	bool AddStackCount();
	
	// Returns if stack is empty or not after reducing 
	bool ReduceStackCount();
	
	bool SetStackCount(int NewStackCount);
	
	bool IsStackFull() const;
	bool IsForItem(const UPA_ShopItem* Item) const;
	bool IsGrantingAbility(TSubclassOf<class UGameplayAbility> AbilityClass) const;
	bool IsGrantingAnyAbility() const;
	
	UInventoryItem();
	bool IsValid() const;
	void InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem, UAbilitySystemComponent* AbilitySystemComponent);
	const UPA_ShopItem* GetShopItem() const {return ShopItem;}
	FInventoryItemHandle GetHandle() const {return Handle;}
	
	bool TryActivateGrantedAbility();
	void ApplyConsumeEffect();
	void RemoveGASModifications();
	FORCEINLINE int GetStackCount() const {return StackCount;}
	void SetSlot(int NewSlot);
	int GetItemSlot() const {return Slot;}
	
	float GetAbilityCooldownTimeRemaining() const;
	float GetAbilityCooldownDuration() const;
	float GetAbilityManaCost() const;
	bool CanCastAbility() const;
	FGameplayAbilitySpecHandle GetGrantedAbilitySpecHandle() const {return GrantedAbilitySpecHandle;}
	void SetGrantedAbilitySpecHandle(FGameplayAbilitySpecHandle SpecHandle) {GrantedAbilitySpecHandle = SpecHandle;};
	
private:
	void ApplyGASModifications();
	UAbilitySystemComponent* OwnerAbilitySystemComponent;
	void ManaUpdated(const FOnAttributeChangeData& ChangeData);
	UPROPERTY()
	const UPA_ShopItem* ShopItem;
	FInventoryItemHandle Handle;
	int StackCount;
	int Slot;
	
	FActiveGameplayEffectHandle AppliedEquipedEffectHandle;
	FGameplayAbilitySpecHandle GrantedAbilitySpecHandle;
};
