using UnrealBuildTool;

public class LogFlowTests : ModuleRules
{
    public LogFlowTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "LogFlowCore",
            "LogFlowEditor"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "AutomationController"
        });
    }
}