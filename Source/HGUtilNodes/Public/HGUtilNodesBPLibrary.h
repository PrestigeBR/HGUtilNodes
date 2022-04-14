// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "HGUtilNodesBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UENUM(BlueprintType, Category = "Weapons")
enum EResourceSizeDisplayMode
{
	EstimatedTotal,
	Exclusive
};

UCLASS()

class UHGUtilNodesBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Copy to Clipboard"), Category = "Hideout Games UtilityNodes|Functions")
	static void CopyToClipboard(FString input);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Clipboard"), Category = "Hideout Games UtilityNodes|Functions")
	static FString GetClipboard();
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "SortABC (String)", Description = "Sort an array alphanumerically.", CompactNodeTitle = "Sort (ABC)", CallableWithoutWorldContext), Category = "Hideout Games UtilityNodes|Functions")
	static TArray<FString> SortAbcString(TArray<FString> Array, bool bInvert);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Object (Pure)", Description = "Sort an array alphanumerically.", CallableWithoutWorldContext, BlueprintAutocast), Category = "Hideout Games UtilityNodes|Functions")
	static UObject* PureConstructObject(UClass* Class);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Object Resource Size as Bytes", Description = "Get the byte size of the object/resource.", CallableWithoutWorldContext, DevelopmentOnly, AdvancedDisplay=2), Category = "Hideout Games UtilityNodes|Functions")
	static uint8 GetResourceSizeBytes(UObject* Object, EResourceSizeDisplayMode OutputMode = EResourceSizeDisplayMode::EstimatedTotal, bool bPrintToScreen = false, bool bPrintToLog = false, FLinearColor TextColor = FLinearColor(255,199,0,1), float Duration = 2.f);
	
};
