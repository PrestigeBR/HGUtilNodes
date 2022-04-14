// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_ForEachMapLoop.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"
#include "Kismet/BlueprintMapLibrary.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/KismetMathLibrary.h"

const FName LoopBodyPinName = "Loop Body";
const FName MapPinName = "Map";
const FName KeyPinName = "Key Element";
const FName ValuePinName = "Value Element";
const FName IndexPinName = "Map Index";
const FName CompletePinName = "Completed";

#define LOCTEXT_NAMESPACE "K2Node_ForEachMapLoop"

void UK2Node_ForEachMapLoop::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	//Exec
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );

	//Input
	FCreatePinParams mapPinParameters;
	mapPinParameters.ContainerType = EPinContainerType::Map;
	mapPinParameters.ValueTerminalType.TerminalCategory = UEdGraphSchema_K2::PC_Wildcard;
	const auto mapPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, MapPinName, mapPinParameters );
	mapPin->PinType.bIsConst = true;
	mapPin->PinType.bIsReference = true;
	wildcardMapType = mapPin->PinType;
	if(!hasInitialized)
	{
		mapCurrentType = wildcardMapType;
		hasInitialized = true;
	}
	
	//Output
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, LoopBodyPinName);

	const auto keyPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, KeyPinName);
	wildcardOutType = keyPin->PinType;

	const auto valuePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, ValuePinName);
	
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Int, IndexPinName);

	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, CompletePinName);

	if(mapCurrentType != wildcardMapType)
	{
		keyPin->PinType = keyCurrentType;
		valuePin->PinType = valueCurrentType;
		mapPin->PinType = mapCurrentType;
	}
}

void UK2Node_ForEachMapLoop::PostPasteNode()
{
	Super::PostPasteNode();

	if(!GetMapPin()->LinkedTo.Num())
	{
		mapCurrentType = wildcardMapType;
		keyCurrentType = wildcardOutType;
		valueCurrentType = wildcardOutType;
	}
}

void UK2Node_ForEachMapLoop::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		//Found errors
		BreakAllNodeLinks();
		return;
	}
	
	const auto K2Schema = GetDefault<UEdGraphSchema_K2>();

	const auto IndexVariableTemp = CompilerContext.SpawnIntermediateNode<UK2Node_TemporaryVariable>(this, SourceGraph);
	IndexVariableTemp->VariableType.PinCategory = UEdGraphSchema_K2::PC_Int;
	IndexVariableTemp->AllocateDefaultPins();

	const auto AssignIndexTemp = CompilerContext.SpawnIntermediateNode<UK2Node_AssignmentStatement>(this, SourceGraph);
	AssignIndexTemp->AllocateDefaultPins();
	K2Schema->TryCreateConnection(AssignIndexTemp->GetVariablePin(), IndexVariableTemp->GetVariablePin());
	AssignIndexTemp->GetValuePin()->DefaultValue = TEXT("0");

	const auto MapKeysNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	MapKeysNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UBlueprintMapLibrary, Map_Keys), UBlueprintMapLibrary::StaticClass());
	MapKeysNode->AllocateDefaultPins();

	const auto ArrayLengthNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	ArrayLengthNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UKismetArrayLibrary, Array_Length), UKismetArrayLibrary::StaticClass());
	ArrayLengthNode->AllocateDefaultPins();
	ArrayLengthNode->FindPinChecked(TEXT("TargetArray"))->PinType.PinCategory = keyCurrentType.PinCategory;
	
	const auto LessThanCompare = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	LessThanCompare->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Less_IntInt), UKismetMathLibrary::StaticClass());
	LessThanCompare->AllocateDefaultPins();

	const auto BranchNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
	BranchNode->AllocateDefaultPins();

	const auto SequenceNode = CompilerContext.SpawnIntermediateNode<UK2Node_ExecutionSequence>(this, SourceGraph);
	SequenceNode->AllocateDefaultPins();

	const auto AssignIndexIncrement = CompilerContext.SpawnIntermediateNode<UK2Node_AssignmentStatement>(this, SourceGraph);
	AssignIndexIncrement->AllocateDefaultPins();

	const auto IncrementIndex = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	IncrementIndex->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Add_IntInt), UKismetMathLibrary::StaticClass());
	IncrementIndex->AllocateDefaultPins();
	IncrementIndex->FindPinChecked(TEXT("B"))->DefaultValue = "1";

	CompilerContext.MovePinLinksToIntermediate(*GetIndexPin(), *IndexVariableTemp->GetVariablePin());
	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *AssignIndexTemp->GetExecPin());
	
	CompilerContext.CopyPinLinksToIntermediate(*GetMapPin(), *MapKeysNode->FindPinChecked(TEXT("TargetMap")));
	K2Schema->TryCreateConnection(MapKeysNode->FindPinChecked(TEXT("Keys")), ArrayLengthNode->FindPinChecked(TEXT("TargetArray")));
	K2Schema->TryCreateConnection(AssignIndexTemp->GetThenPin(), MapKeysNode->GetExecPin());
	K2Schema->TryCreateConnection(MapKeysNode->GetThenPin(), BranchNode->GetExecPin());
	
	K2Schema->TryCreateConnection(IndexVariableTemp->GetVariablePin(), LessThanCompare->FindPinChecked(TEXT("A")));
	K2Schema->TryCreateConnection(ArrayLengthNode->GetReturnValuePin(), LessThanCompare->FindPinChecked(TEXT("B")));
	BranchNode->GetConditionPin()->MakeLinkTo(LessThanCompare->GetReturnValuePin());

	//Loop Body
	K2Schema->TryCreateConnection(BranchNode->GetThenPin(), SequenceNode->GetExecPin());

	const auto MapFindValue = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	MapFindValue->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UBlueprintMapLibrary, Map_Find), UBlueprintMapLibrary::StaticClass());
	MapFindValue->AllocateDefaultPins();
	MapFindValue->FindPinChecked(TEXT("Key"))->PinType.PinCategory = keyCurrentType.PinCategory;
	MapFindValue->FindPinChecked(TEXT("Value"))->PinType.PinCategory = keyCurrentType.PinCategory;

	const auto ArrayGet = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	ArrayGet->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UKismetArrayLibrary, Array_Get), UKismetArrayLibrary::StaticClass());
	ArrayGet->AllocateDefaultPins();
	ArrayGet->FindPinChecked(TEXT("Item"))->PinType.PinCategory = keyCurrentType.PinCategory;
	ArrayGet->FindPinChecked(TEXT("TargetArray"))->PinType.PinCategory = keyCurrentType.PinCategory;
	
	//Seq1
	K2Schema->TryCreateConnection(ArrayGet->FindPinChecked(TEXT("TargetArray")), MapKeysNode->FindPinChecked(TEXT("Keys")));
	K2Schema->TryCreateConnection(IndexVariableTemp->GetVariablePin(), ArrayGet->FindPinChecked(TEXT("Index")));
	CompilerContext.MovePinLinksToIntermediate(*GetKeyPin(), *ArrayGet->FindPinChecked(TEXT("Item")));
	
	CompilerContext.CopyPinLinksToIntermediate(*GetMapPin(), *MapFindValue->FindPinChecked(TEXT("TargetMap")));
	
	K2Schema->TryCreateConnection(MapFindValue->FindPinChecked(TEXT("TargetMap")), GetMapPin());
	K2Schema->TryCreateConnection(ArrayGet->FindPinChecked(TEXT("Item")), MapFindValue->FindPinChecked(TEXT("Key")));
	CompilerContext.MovePinLinksToIntermediate(*GetValuePin(), *MapFindValue->FindPinChecked(TEXT("Value")));

	CompilerContext.CopyPinLinksToIntermediate(*GetLoopBodyPin(), *SequenceNode->GetThenPinGivenIndex(0));
	
	//Seq2
	K2Schema->TryCreateConnection(IncrementIndex->FindPinChecked(TEXT("A")), IndexVariableTemp->GetVariablePin());
	K2Schema->TryCreateConnection(AssignIndexIncrement->GetVariablePin(), IndexVariableTemp->GetVariablePin());
	K2Schema->TryCreateConnection(AssignIndexIncrement->GetValuePin(), IncrementIndex->GetReturnValuePin());
	K2Schema->TryCreateConnection(SequenceNode->GetThenPinGivenIndex(1), AssignIndexIncrement->GetExecPin());
	K2Schema->TryCreateConnection(AssignIndexIncrement->GetThenPin(), BranchNode->GetExecPin());
	
	//Complete
	CompilerContext.MovePinLinksToIntermediate(*GetCompletePin(), *BranchNode->GetElsePin());
	
	BreakAllNodeLinks();
}

void UK2Node_ForEachMapLoop::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (Pin == nullptr) return;

	if (Pin == GetMapPin())
	{
		SetPinTypesInputOutput();
	}
}

bool UK2Node_ForEachMapLoop::CheckForErrors(FKismetCompilerContext& CompilerContext)
{
	bool bError = false;

	if(!GetMapPin()->LinkedTo.Num())
	{
		CompilerContext.MessageLog.Error(*LOCTEXT( "Error", "Node @@ is expecting a Map input." ).ToString( ), this);
		bError = true;
	}

	return bError;
}

void UK2Node_ForEachMapLoop::SetPinTypesInputOutput()
{
	if(GetMapPin()->LinkedTo.Num())
	{
		mapCurrentType = GetMapPin()->LinkedTo[0]->PinType;
		keyCurrentType = FEdGraphPinType::GetTerminalTypeForContainer(GetMapPin()->LinkedTo[0]->PinType);
		valueCurrentType = FEdGraphPinType::GetPinTypeForTerminalType(GetMapPin()->LinkedTo[0]->PinType.PinValueType);
	}
	else
	{
		mapCurrentType = wildcardMapType;
		keyCurrentType = wildcardOutType;
		valueCurrentType = wildcardOutType;
	}
	GetMapPin()->PinType = mapCurrentType;
	GetKeyPin()->PinType = keyCurrentType;
	GetValuePin()->PinType = valueCurrentType;
}

UEdGraphPin* UK2Node_ForEachMapLoop::GetLoopBodyPin() const
{
	return FindPinChecked(LoopBodyPinName);
}

UEdGraphPin* UK2Node_ForEachMapLoop::GetMapPin() const
{
	return FindPinChecked(MapPinName);
}

UEdGraphPin* UK2Node_ForEachMapLoop::GetKeyPin() const
{
	return FindPinChecked(KeyPinName);
}

UEdGraphPin* UK2Node_ForEachMapLoop::GetValuePin() const
{
	return FindPinChecked(ValuePinName);
}

UEdGraphPin* UK2Node_ForEachMapLoop::GetIndexPin() const
{
	return FindPinChecked(IndexPinName);
}

UEdGraphPin* UK2Node_ForEachMapLoop::GetCompletePin() const
{
	return FindPinChecked(CompletePinName);
}

bool UK2Node_ForEachMapLoop::IsNodeSafeToIgnore() const
{
	return true;
}

void UK2Node_ForEachMapLoop::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	if (ActionRegistrar.IsOpenForRegistration(GetClass()))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(GetClass(), NodeSpawner);
	}
}

FText UK2Node_ForEachMapLoop::GetMenuCategory() const
{
	return LOCTEXT( "ForMapNode_Category", "HGUN|K2Nodes" );
}

FText UK2Node_ForEachMapLoop::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT( "ForMapNodeTitle_NONE", "For Each Map Loop" );
}

FText UK2Node_ForEachMapLoop::GetTooltipText() const
{
	return LOCTEXT( "ForMapNode_Tooltip", "Loop through each key of a Map.\n\nTarget is HGUtilNodes." );
}

FSlateIcon UK2Node_ForEachMapLoop::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon( "EditorStyle", "GraphEditor.Macro.ForEach_16x" );
}

FLinearColor UK2Node_ForEachMapLoop::GetNodeTitleColor() const
{
	return FLinearColor(1, 1, 1, 1);
}

#undef LOCTEXT_NAMESPACE
