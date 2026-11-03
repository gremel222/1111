using UnrealBuildTool;

public class CodexOfRulesEditorTarget : TargetRules
{
    public CodexOfRulesEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        CppStandard = CppStandardVersion.Cpp20;
        ExtraModuleNames.Add("CodexOfRules");
    }
}
