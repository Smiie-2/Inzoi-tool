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
            "SlateCore",
            "Json",
            "JsonUtilities"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "RenderCore",
            "RHI"
        });

        // Lua scripting support - uncomment when inZOI's Lua runtime is available
        // PrivateDependencyModuleNames.Add("Lua");
        // PrivateDefinitions.Add("WITH_LUA=1");
        PrivateDefinitions.Add("WITH_LUA=0");

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
