// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameplayWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CanvasPanel.h"
#include "GAS/BaseAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ShopWidget.h"
#include "Widgets/ValueGauge.h"
#include "Widgets/AbilityListView.h"
#include "Widgets/GameplayMenu.h"
#include "GAS/BaseAttributeSet.h"

void UGameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	
	if (OwnerAbilitySystemComponent)
	{
		HealthBar->BindToGameplayAttribute(OwnerAbilitySystemComponent, UBaseAttributeSet::GetHealthAttribute(), UBaseAttributeSet::GetMaxHealthAttribute());
		ManaBar->BindToGameplayAttribute(OwnerAbilitySystemComponent, UBaseAttributeSet::GetManaAttribute(), UBaseAttributeSet::GetMaxManaAttribute());
	}
	
	SetShowMouseCursor(false);
	SetFocusToGameOnly();
	if (GameplayMenu)
	{
		GameplayMenu->GetResumeButtonClickedEventDelegate().AddDynamic(this, &UGameplayWidget::ToggleGameplayMenu);
	}
}

void UGameplayWidget::ConfigureAbilities(const TMap<EAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
	AbilityListView->ConfigureAbilities(Abilities);
}

void UGameplayWidget::ToggleShop()
{
	if (ShopWidget->GetVisibility() == ESlateVisibility::HitTestInvisible)
	{
		ShopWidget->SetVisibility(ESlateVisibility::Visible);
		PlayShopAnim(true);
		SetOwningPawnInputEnabled(false);
		SetShowMouseCursor(true);
		SetFocusToGameAndUI();
		ShopWidget->SetFocus();
	}
	else
	{
		ShopWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		PlayShopAnim(false);
		SetOwningPawnInputEnabled(true);
		SetShowMouseCursor(false);
		SetFocusToGameOnly();
	}
}

void UGameplayWidget::ToggleGameplayMenu()
{
	if (MainSwitcher->GetActiveWidget() == GameplayMenuRootPanel)
	{
		MainSwitcher->SetActiveWidget(GameplayWidgetRootPanel);
		SetOwningPawnInputEnabled(true);
		SetShowMouseCursor(false);
		SetFocusToGameOnly();
	}
	else
	{
		ShowGameplayMenu();
	}
}

void UGameplayWidget::ShowGameplayMenu()
{
	MainSwitcher->SetActiveWidget(GameplayMenuRootPanel);
	SetOwningPawnInputEnabled(false);
	SetShowMouseCursor(true);
	SetFocusToGameAndUI();
}

void UGameplayWidget::SetGameplayMenuTitle(const FString& NewTitle)
{
	GameplayMenu->SetTitleText(NewTitle);
}

void UGameplayWidget::PlayShopAnim(bool bPlayForward)
{
	if (bPlayForward)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayShopAnim: Playing Shop"));
		PlayAnimationForward(ShopPopUp);
	}
	else
	{
		PlayAnimationReverse(ShopPopUp);
	}
}

void UGameplayWidget::SetOwningPawnInputEnabled(bool bEnablePawnInput)
{
	if (bEnablePawnInput)
	{
		GetOwningPlayerPawn()->EnableInput(GetOwningPlayer());
	}
	else
	{
		GetOwningPlayerPawn()->DisableInput(GetOwningPlayer());
	}
}

void UGameplayWidget::SetShowMouseCursor(bool bShowMouseCursor)
{
	GetOwningPlayer()->SetShowMouseCursor(bShowMouseCursor);
}

void UGameplayWidget::SetFocusToGameAndUI()
{
	FInputModeGameAndUI GameAndUIInputMode;
	GameAndUIInputMode.SetHideCursorDuringCapture(false);
	
	GetOwningPlayer()->SetInputMode(GameAndUIInputMode);
}

void UGameplayWidget::SetFocusToGameOnly()
{
	FInputModeGameOnly GameOnlyInputMode;
	GetOwningPlayer()->SetInputMode(GameOnlyInputMode);
}
