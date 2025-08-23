using UnrealBuildTool;
using System.Collections.Generic;

public class BikeAdventureEditorTarget : TargetRules
{
	public BikeAdventureEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

		ExtraModuleNames.AddRange(new string[] { "BikeAdventure" });

		// Editor-specific optimizations
		bUseUnityBuild = false; // Better for iteration in editor
		bUsePCHFiles = true;
		bIWYU = true;
		
		// Enable modern C++ features
		CppStandardVersion = CppStandardVersion.Cpp20;

		// Editor features
		bWithLiveCoding = true;
		bCompileWithStatsDisplay = true;
		bCompileWithProfiler = true;
		
		// Development features
		bUseLoggingInShipping = false;
		bUseChecksInShipping = false;
		bUseFastMonoCalls = true;

		// Enable editor-specific plugins and features
		bBuildDeveloperTools = true;
		bBuildWithEditorOnlyData = true;
		bCompileAgainstEngine = true;
		bCompileAgainstCoreUObject = true;
	}
}