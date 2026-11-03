using UnrealBuildTool;

public class CodexOfRulesTarget : TargetRules
{
    public CodexOfRulesTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        CppStandard = CppStandardVersion.Cpp20;
        ExtraModuleNames.Add("CodexOfRules");
    }
}
