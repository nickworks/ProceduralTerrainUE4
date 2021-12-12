// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectCPP : ModuleRules
{
	public ProjectCPP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"HeadMountedDisplay",
			"ProceduralMeshComponent",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "ProceduralMeshComponent" });
	}
}
