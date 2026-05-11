using UnrealBuildTool;

public class LogFlowBlueprintLibrary : ModuleRules
{
    public LogFlowBlueprintLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "LogFlowCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}