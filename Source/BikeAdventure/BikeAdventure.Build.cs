using UnrealBuildTool;

public class BikeAdventure : ModuleRules
{
	public BikeAdventure(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"PhysicsCore",
			"Chaos",
			"ChaosVehicles"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Slate", 
			"SlateCore",
			"UnrealEd",
			"ToolMenus",
			"EditorStyle",
			"EditorWidgets",
			"GameplayTags"
		});

		// For automated testing
		if (Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { 
				"AutomationController", 
				"AutomationWindow", 
				"AutomationMessages"
			});
		}
	}
}