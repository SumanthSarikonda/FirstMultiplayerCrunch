// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ValueGauge.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "Components/TextBlock.h"

#include "DebugHelper.h"

void UValueGauge::NativePreConstruct()
{
	Super::NativePreConstruct();
	ProgressBar->SetFillColorAndOpacity(BarColor);
	
	ValueText->SetFont(ValueTextFont);
	
	ValueText->SetVisibility(bValueTextVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	ProgressBar->SetVisibility(bProgressBarVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UValueGauge::BindToGameplayAttribute(class UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute)
{
	if (AbilitySystemComponent)
	{
		bool bFound;
		float Value = AbilitySystemComponent->GetGameplayAttributeValue(Attribute, bFound);
		float MaxValue = AbilitySystemComponent->GetGameplayAttributeValue(MaxAttribute, bFound);
		if (bFound)
		{
			SetValue(Value, MaxValue);
		}
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UValueGauge::ValueChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UValueGauge::MaxValueChanged);
	}
}

void UValueGauge::SetValue(float NewValue, float NewMaxValue)
{
	CachedValue = NewValue;
	CachedMaxValue = NewMaxValue;
	
	if (NewMaxValue == 0)
	{
		return;
	}
	
	float NewPercent = NewValue / NewMaxValue;
	ProgressBar->SetPercent(NewPercent);
	
	FNumberFormattingOptions FormattingOptions = FNumberFormattingOptions().SetMaximumFractionalDigits(0);
	
	ValueText->SetText(
		FText::Format(
			FTextFormat::FromString("{0}/{1}"),
			FText::AsNumber(NewValue, &FormattingOptions),
			FText::AsNumber(NewMaxValue, &FormattingOptions)
		)
	);
}

void UValueGauge::ValueChanged(const FOnAttributeChangeData& Data)
{
	SetValue(Data.NewValue, CachedMaxValue);
}

void UValueGauge::MaxValueChanged(const FOnAttributeChangeData& Data)
{
	SetValue( CachedValue,Data.NewValue);
}
