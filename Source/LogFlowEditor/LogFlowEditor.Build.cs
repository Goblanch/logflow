using UnrealBuildTool;

public class LogFlowEditor : ModuleRules
{
    public LogFlowEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "Slate",
            "SlateCore",
            "LogFlowCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}