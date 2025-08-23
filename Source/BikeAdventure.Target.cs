using UnrealBuildTool;
using System.Collections.Generic;

public class BikeAdventureTarget : TargetRules
{
	public BikeAdventureTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

		ExtraModuleNames.AddRange(new string[] { "BikeAdventure" });

		// Performance optimizations for the meditative bike game
		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			// Optimize for performance in shipping builds
			bUseUnityBuild = true;
			bUsePCHFiles = true;
			MinFilesUsingPrecompiledHeaderOverride = 1;
			bIWYU = false;
		}
		else
		{
			// Faster iteration in development builds
			bUseUnityBuild = false;
			bUsePCHFiles = true;
			bIWYU = true;
		}

		// Enable modern C++ features
		CppStandardVersion = CppStandardVersion.Cpp20;
		
		// Disable unnecessary features for better performance
		bWithLiveCoding = false;
		bUseLoggingInShipping = false;
		bUseChecksInShipping = false;
		bUseFastMonoCalls = true;

		// Enable automation testing
		if (Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			bCompileWithStatsDisplay = true;
			bCompileWithProfiler = true;
		}
	}
}