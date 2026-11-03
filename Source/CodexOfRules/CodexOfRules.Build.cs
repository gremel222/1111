using UnrealBuildTool;

public class CodexOfRules : ModuleRules
{
    public CodexOfRules(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore" });
        // SimulationCore headers/source live in this module but never include Unreal headers.
        // The same C++17-compatible source is compiled by tools/build_core.py for native tests.
    }
}
