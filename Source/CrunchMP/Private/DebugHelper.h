#pragma once

namespace  Debug
{
	static void print(const FString& MSG, const FColor Color = FColor::MakeRandomColor(), int32 Inkey = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(Inkey, 7.f, Color, MSG);
		
			UE_LOG(LogTemp, Warning, TEXT("%s"), *MSG);
		}
	}
	
	static void print(const FString& FloatTitle, float FloatValueToPrint, int32 Inkey = -1, const FColor Color = FColor::MakeRandomColor())
	{
		if (GEngine)
		{
			const FString FinalMSG = FloatTitle + TEXT(": ") + FString::SanitizeFloat(FloatValueToPrint);
			GEngine->AddOnScreenDebugMessage(Inkey, 7.f, Color, FinalMSG);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FinalMSG);
		}
	}
}

