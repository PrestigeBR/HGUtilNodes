#pragma once
#include "K2Node.h"
#include "K2Node_AddPinInterface.h"

#include "K2Node_PrintAny.generated.h"

UCLASS()
class CUSTOMK2_API UK2Node_PrintAny : public UK2Node, public IK2Node_AddPinInterface
{
	GENERATED_BODY()

public:
	
	virtual void AllocateDefaultPins() override;

	virtual void PostPasteNode() override;

	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	//Pin Getters
	UE_NODISCARD UEdGraphPin* GetWildcardPin(const int32 id) const;

	UE_NODISCARD UEdGraphPin* GetThenPin(void) const;
	
	//Setup
	virtual bool IsNodeSafeToIgnore() const override;

	virtual bool ShouldShowNodeProperties() const override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;

	virtual FText GetMenuCategory() const override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;
	
	virtual FLinearColor GetNodeTitleColor() const override;

	UE_NODISCARD void RemoveLastPin();

	UE_NODISCARD void AddNewPin();
	
	UPROPERTY() int32 pinAmount = 0;

	virtual void AddInputPin() override;

	UE_NODISCARD void RemoveInputPin(UEdGraphPin* pin);

private:

	//Pin Name
	//static const FName thenExecPinName;

	//Error check
	UE_NODISCARD bool CheckForErrors(FKismetCompilerContext& CompilerContext);
	
	//Pin data
	UPROPERTY() TArray<FEdGraphPinType> currentPinTypes;

	UPROPERTY() FEdGraphPinType wildcardPinType;
	
};