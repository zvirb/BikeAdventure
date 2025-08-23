// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BikeAdventureTests : ModuleRules
{
	public BikeAdventureTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"UnrealEd",
			"AutomationController", 
			"AutomationWorker", 
			"Gauntlet",
			"BikeAdventure" // Main game module
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate", 
			"SlateCore", 
			"ToolMenus", 
			"EditorStyle",
			"EditorWidgets", 
			"DesktopWidgets",
			"FunctionalTesting",
			"ScreenShotComparison",
			"AutomationMessages",
			"SessionServices",
			"SourceControl",
			"TargetPlatform"
		});

		// Enable automation testing
		PublicDefinitions.Add("WITH_DEV_AUTOMATION_TESTS=1");
		PublicDefinitions.Add("WITH_PERF_AUTOMATION_TESTS=1");
	}
}