using UnrealBuildTool;

public class CustomK2 : ModuleRules
{
	public CustomK2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"InputCore",
			"Slate",
			"Engine",
			"AssetTools",
			"UnrealEd",
			"KismetWidgets",
			"KismetCompiler",
			"BlueprintGraph",
			"GraphEditor",
			"Kismet",
			"PropertyEditor",
			"EditorStyle",
			"Slate",
			"SlateCore",
			"Sequencer",
			"DetailCustomizations",
			"Settings",
			"RenderCore",
			"ToolMenus",
		});
	}
}
