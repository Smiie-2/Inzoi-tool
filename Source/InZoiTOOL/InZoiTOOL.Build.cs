using UnrealBuildTool;

public class InZoiTOOL : ModuleRules
{
    public InZoiTOOL(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Slate",
            "SlateCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "RenderCore",
            "RHI",
            "Lua"
        });

        PublicIncludePaths.AddRange(new string[]
        {
            "InZoiTOOL/Public"
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "InZoiTOOL/Private"
        });
    }
}
