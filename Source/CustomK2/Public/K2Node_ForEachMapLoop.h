// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "K2Node_ForEachMapLoop.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMK2_API UK2Node_ForEachMapLoop : public UK2Node
{
	GENERATED_BODY()

public:

	virtual void AllocateDefaultPins() override;

	virtual void PostPasteNode() override;

	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	UE_NODISCARD bool CheckForErrors(FKismetCompilerContext& CompilerContext);

	UE_NODISCARD void SetPinTypesInputOutput();

	//Pins
	UE_NODISCARD UEdGraphPin* GetLoopBodyPin() const;
	UE_NODISCARD UEdGraphPin* GetMapPin() const;
	UE_NODISCARD UEdGraphPin* GetKeyPin() const;
	UE_NODISCARD UEdGraphPin* GetValuePin() const;
	UE_NODISCARD UEdGraphPin* GetIndexPin() const;
	UE_NODISCARD UEdGraphPin* GetCompletePin() const;

	UPROPERTY() bool hasInitialized;
	UPROPERTY() FEdGraphPinType keyCurrentType;
	UPROPERTY() FEdGraphPinType valueCurrentType;
	UPROPERTY() FEdGraphPinType mapCurrentType;
	UPROPERTY() FEdGraphPinType wildcardMapType;
	UPROPERTY() FEdGraphPinType wildcardOutType;

	//Setup
	virtual bool IsNodeSafeToIgnore() const override;

	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

	virtual FText GetMenuCategory() const override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;
	
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;

	virtual FLinearColor GetNodeTitleColor() const override;
	
};
