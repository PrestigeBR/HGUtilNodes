#include "CustomK2/Public/K2Node_PrintAny.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"

#include "EdGraphSchema_K2.h"
#include "K2Node_FormatText.h"
#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "Kismet/KismetSystemLibrary.h"

#include "KismetCompiler.h"
#include "ToolMenu.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "K2Node_PrintAny"

void UK2Node_PrintAny::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	//Exec
	CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute );

	//Then
	CreatePin( EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	
	//Input
	for(int32 i=0; i<=pinAmount; i++)
	{
		const FString name = "Wildcard_" + FString::FromInt(i);
		const auto AnyPin = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, FName(name));
		AnyPin->PinFriendlyName = FText::FromName(AnyPin->PinType.PinCategory);
		wildcardPinType = AnyPin->PinType;
		if(currentPinTypes.Num() <= i)
		{
			currentPinTypes.Add(wildcardPinType);
		}
		else if(currentPinTypes[i].PinCategory  == NAME_None)
		{
			currentPinTypes[i] = wildcardPinType;
		}
		else if (currentPinTypes[i].PinCategory != UEdGraphSchema_K2::PC_Wildcard)
		{
			AnyPin->PinType = currentPinTypes[i];
			AnyPin->PinType.PinCategory = currentPinTypes[i].PinCategory;
			AnyPin->PinFriendlyName = FText::FromName(currentPinTypes[i].PinCategory);
		}
		const FString tooltip = "Wildcard(" + currentPinTypes[i].PinCategory.ToString() + ") " + FString::FromInt(i) + " input to print.";
		AnyPin->PinToolTip = tooltip;
	}

	//Advanced options
	const auto pToScreen = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Boolean, "bPrintToScreen");
	pToScreen->bAdvancedView = true;
	pToScreen->DefaultValue = "true";

	const auto pToLog = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Boolean, "bPrintToLog");
	pToLog->bAdvancedView = true;
	pToLog->DefaultValue = "true";

	const auto duration = CreatePin( EGPD_Input, UEdGraphSchema_K2::PC_Float, "Duration");
	duration->bAdvancedView = true;
	duration->DefaultValue = "2.0";

	SetEnabledState(ENodeEnabledState::DevelopmentOnly, false);

	if (AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
}

void UK2Node_PrintAny::PostPasteNode()
{
	Super::PostPasteNode( );

	for(int32 i=0; i<=pinAmount; i++)
	{
		currentPinTypes[i].PinCategory = NAME_None;
		if (const auto AnyPin = GetWildcardPin(i))
		{
			if (currentPinTypes[i].PinCategory == NAME_None && AnyPin->LinkedTo.Num())
			{
				PinConnectionListChanged(AnyPin);
			}
		}	
	}
}

void UK2Node_PrintAny::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckForErrors( CompilerContext ))
	{
		//Found errors
		BreakAllNodeLinks( );
		return;
	}

	TArray<UK2Node_CallFunction*> printPins;
	
	for(int32 i=0; i<=pinAmount; i++)
	{
		bool bAddedToList = false;
		
		const auto PrintNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>( this, SourceGraph );
		CompilerContext.MessageLog.NotifyIntermediateObjectCreation(PrintNode, this);
	
		const FName PrintTextFunc = GET_FUNCTION_NAME_CHECKED( UKismetSystemLibrary, PrintString );
		PrintNode->FunctionReference.SetExternalMember( PrintTextFunc, UKismetSystemLibrary::StaticClass( ) );
		PrintNode->AllocateDefaultPins();

		const auto printInStringPin = PrintNode->FindPinChecked( TEXT( "InString" ) );

		//ToString
		const auto convNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>( this, SourceGraph );

		//Bool
		if (GetWildcardPin(i)->PinType.PinCategory == UEdGraphSchema_K2::PC_Boolean) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_BoolToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InBool")));
			bAddedToList = true;
		}

		//Byte
		if (GetWildcardPin(i)->PinType.PinCategory == UEdGraphSchema_K2::PC_Byte) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_ByteToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InByte")));
			bAddedToList = true;
		}

		//Int
		if (GetWildcardPin(i)->PinType.PinCategory == UEdGraphSchema_K2::PC_Int) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_IntToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InInt")));
			bAddedToList = true;
		}

		//Float
		if (GetWildcardPin(i)->PinType.PinCategory == UEdGraphSchema_K2::PC_Float) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_FloatToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InFloat")));
			bAddedToList = true;
		}

		//Vector
		if (GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() != nullptr && GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() == TBaseStructure<FVector>::Get()) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_VectorToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InVec")));
			bAddedToList = true;
		}

		//Transform
		if (GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() != nullptr && GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() == TBaseStructure<FTransform>::Get()) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_TransformToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InTrans")));
			bAddedToList = true;
		}

		//Object
		if (GetWildcardPin(i)->PinType.PinCategory == UEdGraphSchema_K2::PC_Object) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_ObjectToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InObj")));
			bAddedToList = true;
		}

		//LinearColor
		if (GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() != nullptr && GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() == TBaseStructure<FLinearColor>::Get()) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_ColorToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InColor")));
			bAddedToList = true;
		}

		//Name
		if (GetWildcardPin(i)->PinType.PinCategory == UEdGraphSchema_K2::PC_Name) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_NameToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InName")));
			bAddedToList = true;
		}

		//Rotator
		if (GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() != nullptr && GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() == TBaseStructure<FRotator>::Get()) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_RotatorToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InRot")));
			bAddedToList = true;
		}

		//Vector2D
		if (GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() != nullptr && GetWildcardPin(i)->PinType.PinSubCategoryObject.Get() == TBaseStructure<FVector2D>::Get()) {
			const FName convFunc = GET_FUNCTION_NAME_CHECKED( UKismetStringLibrary, Conv_Vector2dToString );
			convNode->FunctionReference.SetExternalMember( convFunc, UKismetStringLibrary::StaticClass( ) );
			convNode->AllocateDefaultPins();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *convNode->FindPinChecked(TEXT("InVec")));
			bAddedToList = true;
		}

		//String
		if (GetWildcardPin(i)->PinType.PinCategory == UEdGraphSchema_K2::PC_String) {
			convNode->DestroyNode();
			CompilerContext.MovePinLinksToIntermediate(*GetWildcardPin(i), *printInStringPin);
		}
		else
		{
			if(!bAddedToList)
			{
				CompilerContext.MessageLog.Error(*LOCTEXT( "Error", "Node @@ has a invalid input type." ).ToString( ), this);
				return;
			}
			convNode->FindPin(TEXT("ReturnValue"))->MakeLinkTo(printInStringPin);
		}

		//const UObject* WorldContextObject, const FString& InString, bool bPrintToScreen, bool bPrintToLog, FLinearColor TextColor, float Duration)
		//Print options
		if(FindPin(FName("bPrintToScreen"))->LinkedTo.Num()) {
			FindPin(FName("bPrintToScreen"))->MakeLinkTo(PrintNode->FindPin(FName("bPrintToScreen")));
		}else{
			PrintNode->FindPin(FName("bPrintToScreen"))->DefaultValue = FindPin(FName("bPrintToScreen"))->DefaultValue;
		}
		if(FindPin(FName("bPrintToLog"))->LinkedTo.Num()) {
			FindPin(FName("bPrintToLog"))->MakeLinkTo(PrintNode->FindPin(FName("bPrintToLog")));
		}else{
			PrintNode->FindPin(FName("bPrintToLog"))->DefaultValue = FindPin(FName("bPrintToLog"))->DefaultValue;
		}
		if(FindPin(FName("Duration"))->LinkedTo.Num()) {
			FindPin(FName("Duration"))->MakeLinkTo(PrintNode->FindPin(FName("Duration")));
		}else{
			PrintNode->FindPin(FName("Duration"))->DefaultValue = FindPin(FName("Duration"))->DefaultValue;
		}

		printPins.Add(PrintNode);

		if(i != 0 || i != printPins.Num()-1)
		{
			printPins[i-1]->GetThenPin()->MakeLinkTo(PrintNode->GetExecPin());
		}
	}
	
	UEdGraphPin* InternalExec = printPins[0]->GetExecPin();
	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *InternalExec);

	UEdGraphPin* InternalThen = printPins[printPins.Num()-1]->GetThenPin();
	CompilerContext.MovePinLinksToIntermediate(*FindPin(UEdGraphSchema_K2::PN_Then), *InternalThen);	

	BreakAllNodeLinks();
}

void UK2Node_PrintAny::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (Pin == nullptr)
		return;

	for (int32 i=0; i<=pinAmount; i++)
	{
		const FString pinName = "Wildcard_" + FString::FromInt(i);
		if (Pin->PinName == FName(*pinName))
		{
			if(currentPinTypes.Num() <= i)
			{
				if (Pin->LinkedTo.Num())
					{ currentPinTypes.Add(Pin->LinkedTo[ 0 ]->PinType); Pin->PinFriendlyName = FText::FromName(Pin->LinkedTo[0]->PinType.PinCategory); }
				else
					{ currentPinTypes.Add(wildcardPinType); Pin->PinFriendlyName = FText::FromName(currentPinTypes[i].PinCategory); }
			}else
			{
				if (Pin->LinkedTo.Num())
					{ currentPinTypes[i] = Pin->LinkedTo[ 0 ]->PinType; Pin->PinFriendlyName = FText::FromName(Pin->LinkedTo[0]->PinType.PinCategory); }
				else
					{ currentPinTypes[i] = wildcardPinType; Pin->PinFriendlyName = FText::FromName(currentPinTypes[i].PinCategory); }
			}

			Pin->PinType = currentPinTypes[i];
			if(Pin->PinType == wildcardPinType)
			{
				currentPinTypes[i].PinCategory = NAME_None;
			}

			const FString tooltip = "Wildcard(" + currentPinTypes[i].PinCategory.ToString() + ") " + FString::FromInt(i) + " input to print.";
			
			Pin->PinToolTip = tooltip;
		}	
	}
}

void UK2Node_PrintAny::AddInputPin()
{
	Modify();
	AddNewPin();
	ReconstructNode();
}

void UK2Node_PrintAny::RemoveInputPin(UEdGraphPin* pin)
{
	if(Pins.Contains(pin) && pin->ParentPin == nullptr)
	{
		Modify();
		Pins.RemoveAt(Pins.Find(pin));
		pin->MarkPendingKill();
		PinConnectionListChanged(pin);
		pinAmount--;
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());	
	}
}

bool UK2Node_PrintAny::CheckForErrors(FKismetCompilerContext& CompilerContext)
{
	bool bError = false;

	for(int32 i=0; i<=pinAmount; i++)
	{
		if(GetWildcardPin(i) == nullptr)
		{
			CompilerContext.MessageLog.Error(*LOCTEXT( "Error", "Node @@ has a nullptr wildcard input." ).ToString( ), this);
			bError = true;
		}
		if(GetWildcardPin(i)->LinkedTo.Num() == 0)
		{
			CompilerContext.MessageLog.Error(*LOCTEXT( "Error", "Node @@ is missing a wildcard input." ).ToString( ), this);
			bError = true;
		}	
	}

	return bError;
}

//Pin Getters
UEdGraphPin* UK2Node_PrintAny::GetWildcardPin(const int32 id) const
{
	const FString pinName = "Wildcard_" + FString::FromInt(id);
	return FindPin(FName(*pinName));
}
UEdGraphPin* UK2Node_PrintAny::GetThenPin() const
{
	return FindPinChecked(UEdGraphSchema_K2::PN_Then);
}

//Setup
bool UK2Node_PrintAny::IsNodeSafeToIgnore() const
{
	return true;
}

bool UK2Node_PrintAny::ShouldShowNodeProperties() const
{
	return true;
}

void UK2Node_PrintAny::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	ReconstructNode();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UK2Node_PrintAny::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	//CustomK2Utilities::RegisterAction(ActionRegistrar, GetClass());
	if (ActionRegistrar.IsOpenForRegistration(GetClass()))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(GetClass(), NodeSpawner);
	}
}

void UK2Node_PrintAny::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	FToolMenuSection& Section = Menu->AddSection("K2NodePrintAny", NSLOCTEXT("K2Nodes", "PrintAnyHeader", "PrintAny"));

	if (Context->Pin != NULL)
	{
		if (Context->Pin->Direction == EGPD_Input && Context->Pin->ParentPin == nullptr)
		{
			Section.AddMenuEntry(
				"RemovePin",
				LOCTEXT("RemovePin", "Remove this input pin"),
				LOCTEXT("RemovePinTooltip", "Remove this input pin"),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateUObject(const_cast<UK2Node_PrintAny*>(this), &UK2Node_PrintAny::RemoveInputPin, const_cast<UEdGraphPin*>(Context->Pin))
				)
			);
		}
	}
	
	Section.AddMenuEntry(
		"RemoveLastPin",
		LOCTEXT("RemoveLastPin", "Remove the last input pin"),
		LOCTEXT("RemoveLastPinTooltip", "Remove the last input pin"),
		FSlateIcon(),
		FUIAction(
		FExecuteAction::CreateUObject(const_cast<UK2Node_PrintAny*>(this), &UK2Node_PrintAny::RemoveLastPin)
		)
	);

}

FText UK2Node_PrintAny::GetMenuCategory() const
{
	return LOCTEXT( "NodeCategory", "HGUN|K2Nodes" );
}

FText UK2Node_PrintAny::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT( "NodeTitle_NONE", "Print Any" );
}

FText UK2Node_PrintAny::GetTooltipText() const
{
	return LOCTEXT( "NodeTooltip", "Print any wildcard value to screen or log.\n\nTarget is HGUtilNodes." );
}

FLinearColor UK2Node_PrintAny::GetNodeTitleColor() const
{
	return FLinearColor(0, 0, 0, 1);
}

void UK2Node_PrintAny::RemoveLastPin()
{
	pinAmount = pinAmount-1;
	ReconstructNode();
}

void UK2Node_PrintAny::AddNewPin()
{
	pinAmount = pinAmount+1;
}

#undef LOCTEXT_NAMESPACE
