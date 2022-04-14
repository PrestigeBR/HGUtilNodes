// Copyright Epic Games, Inc. All Rights Reserved.

#include "HGUtilNodesBPLibrary.h"
#include "HGUtilNodes.h"
#include "Kismet/GameplayStatics.h"

UHGUtilNodesBPLibrary::UHGUtilNodesBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UHGUtilNodesBPLibrary::CopyToClipboard(FString input)
{
	FPlatformMisc::ClipboardCopy(*input);
}

FString UHGUtilNodesBPLibrary::GetClipboard()
{
	FString output;
	FPlatformMisc::ClipboardPaste(output);
	return output;
}

TArray<FString> UHGUtilNodesBPLibrary::SortAbcString(TArray<FString> Array, bool bInvert)
{
	TArray<FString> Output = Array;
	Output.Sort();
	if(bInvert)
	{
		Algo::Reverse(Output);
	}
	return Output;
}

UObject* UHGUtilNodesBPLibrary::PureConstructObject(UClass* Class)
{
	UObject* output = NewObject<UObject>(nullptr, Class);
	return output;
}

uint8 UHGUtilNodesBPLibrary::GetResourceSizeBytes(UObject* Object, EResourceSizeDisplayMode OutputMode,
	bool bPrintToScreen, bool bPrintToLog, FLinearColor TextColor, float Duration)
{
	if(!Object) return 0;
	
	uint8 usage;
	
	switch (OutputMode)
	{
	case EResourceSizeDisplayMode::Exclusive:
		usage = Object->GetResourceSizeBytes(EResourceSizeMode::Exclusive);
		break;
	case EResourceSizeDisplayMode::EstimatedTotal:
		usage = Object->GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal);
		break;
	default:
		usage = Object->GetResourceSizeBytes(EResourceSizeMode::Exclusive);
		break;
	}
	
	if(bPrintToLog)
	{
		const FString objName = Object->GetName();
		UE_LOG(LogTemp, Warning, TEXT("%s is using: %i bytes"), *objName, usage);
	}

	if(bPrintToScreen)
	{
		const FString objName = Object->GetName();
		const FColor Color = TextColor.ToFColor(false);
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, FString::Printf(TEXT("%s is using: %i bytes"), *objName, usage));
	}
	
	return usage;
}


