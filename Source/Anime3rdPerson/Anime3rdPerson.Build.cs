// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Anime3rdPerson : ModuleRules
{
	public Anime3rdPerson(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
