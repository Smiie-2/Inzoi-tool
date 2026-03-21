// ============================================================================
// InZoi T.O.O.L. - ImGui GUI Companion Mod (C++ UE4SS CppMod)
//
// This C++ mod registers an ImGui tab in the UE4SS debug window that provides
// a visual GUI for the T.O.O.L. Lua mod. It communicates with the Lua mod
// via UE4SS shared variables.
//
// Build: Compile as a UE4SS CppMod DLL, place in InZoiTOOL_GUI/dlls/main.dll
// See: https://docs.ue4ss.com/guides/creating-a-c++-mod.html
// ============================================================================

#define NOMINMAX
#include <Mod/CppUserModBase.hpp>
#include <UE4SSProgram.hpp>
#include <imgui.h>
#include <string>
#include <vector>
#include <cstring>

// Shared variable keys used by the Lua mod
// The Lua mod writes these, the C++ mod reads them for display
static constexpr const char* SV_TOOL_ACTIVE = "TOOL_Active";
static constexpr const char* SV_MODE = "TOOL_Mode";
static constexpr const char* SV_AXIS = "TOOL_Axis";
static constexpr const char* SV_SELECTED_NAME = "TOOL_SelectedName";
static constexpr const char* SV_POS_X = "TOOL_PosX";
static constexpr const char* SV_POS_Y = "TOOL_PosY";
static constexpr const char* SV_POS_Z = "TOOL_PosZ";
static constexpr const char* SV_ROT_P = "TOOL_RotP";
static constexpr const char* SV_ROT_Y = "TOOL_RotY";
static constexpr const char* SV_ROT_R = "TOOL_RotR";
static constexpr const char* SV_SCALE_X = "TOOL_ScaleX";
static constexpr const char* SV_SCALE_Y = "TOOL_ScaleY";
static constexpr const char* SV_SCALE_Z = "TOOL_ScaleZ";
static constexpr const char* SV_UNDO_COUNT = "TOOL_UndoCount";
static constexpr const char* SV_REDO_COUNT = "TOOL_RedoCount";
static constexpr const char* SV_STATUS = "TOOL_Status";

// Command channel: C++ writes, Lua reads and executes
static constexpr const char* SV_CMD = "TOOL_Command";

class InZoiTOOL_GUI : public RC::CppUserModBase
{
public:
    InZoiTOOL_GUI()
    {
        ModName = STR("InZoiTOOL_GUI");
        ModVersion = STR("1.0.0");
        ModDescription = STR("ImGui GUI for InZoi T.O.O.L.");
    }

    ~InZoiTOOL_GUI() override = default;

    auto on_program_start() -> void override
    {
        // Register our ImGui tab in the UE4SS debug window
        register_tab(STR("T.O.O.L."), [this](){ render_tab(); });
    }

    auto on_unreal_init() -> void override {}

private:
    // Buffers for ImGui input
    char classFilterBuf[128] = "Actor";
    char coordInputBuf[256] = "";
    char macroNameBuf[64] = "";
    bool showCoordInput = false;
    bool showHelp = false;
    bool showSettings = false;

    auto send_command(const std::string& cmd) -> void
    {
        // Write command to shared variable for Lua to pick up
        set_shared_variable(SV_CMD, cmd);
    }

    auto get_shared_string(const char* key, const char* fallback = "") -> std::string
    {
        auto val = get_shared_variable(key);
        return val.empty() ? std::string(fallback) : val;
    }

    auto get_shared_float(const char* key, float fallback = 0.0f) -> float
    {
        auto val = get_shared_variable(key);
        if (val.empty()) return fallback;
        try { return std::stof(val); }
        catch (...) { return fallback; }
    }

    auto get_shared_int(const char* key, int fallback = 0) -> int
    {
        auto val = get_shared_variable(key);
        if (val.empty()) return fallback;
        try { return std::stoi(val); }
        catch (...) { return fallback; }
    }

    auto render_tab() -> void
    {
        auto isActive = get_shared_string(SV_TOOL_ACTIVE, "false");
        auto mode = get_shared_string(SV_MODE, "move");
        auto axis = get_shared_string(SV_AXIS, "free");
        auto selectedName = get_shared_string(SV_SELECTED_NAME, "");
        auto status = get_shared_string(SV_STATUS, "");

        // Active toggle
        bool active = (isActive == "true");
        if (ImGui::Checkbox("T.O.O.L. Active (F2)", &active))
        {
            send_command("toggle");
        }

        if (!active)
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                "Press F2 or check the box to activate");
            return;
        }

        ImGui::Separator();

        // Mode selector
        ImGui::Text("Mode:");
        ImGui::SameLine();
        const char* modes[] = {"move", "rotate", "scale", "elevate"};
        const char* modeNames[] = {"MOVE", "ROTATE", "SCALE", "ELEVATE"};
        for (int i = 0; i < 4; i++)
        {
            if (i > 0) ImGui::SameLine();
            bool isMode = (mode == modes[i]);
            if (isMode) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.0f, 1.0f));
            if (ImGui::Button(modeNames[i]))
            {
                send_command(std::string("mode:") + modes[i]);
            }
            if (isMode) ImGui::PopStyleColor();
        }

        // Axis selector
        ImGui::Text("Axis:");
        ImGui::SameLine();
        const char* axes[] = {"free", "x", "y", "z", "xy", "xz", "yz"};
        const char* axisNames[] = {"FREE", "X", "Y", "Z", "XY", "XZ", "YZ"};
        for (int i = 0; i < 7; i++)
        {
            if (i > 0) ImGui::SameLine();
            bool isAxis = (axis == axes[i]);
            if (isAxis) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.0f, 1.0f));
            if (ImGui::Button(axisNames[i]))
            {
                send_command(std::string("axis:") + axes[i]);
            }
            if (isAxis) ImGui::PopStyleColor();
        }

        ImGui::Separator();

        // Selected object display
        if (!selectedName.empty())
        {
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
                "Selected: %s", selectedName.c_str());

            auto px = get_shared_float(SV_POS_X);
            auto py = get_shared_float(SV_POS_Y);
            auto pz = get_shared_float(SV_POS_Z);
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "X:%.2f", px);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Y:%.2f", py);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.3f, 0.4f, 1.0f, 1.0f), "Z:%.2f", pz);

            auto rp = get_shared_float(SV_ROT_P);
            auto ry = get_shared_float(SV_ROT_Y);
            auto rr = get_shared_float(SV_ROT_R);
            ImGui::Text("Rot: P:%.1f  Y:%.1f  R:%.1f", rp, ry, rr);

            auto sx = get_shared_float(SV_SCALE_X, 1.0f);
            auto sy = get_shared_float(SV_SCALE_Y, 1.0f);
            auto sz = get_shared_float(SV_SCALE_Z, 1.0f);
            ImGui::Text("Scale: X:%.2f  Y:%.2f  Z:%.2f", sx, sy, sz);

            ImGui::Separator();

            // Action buttons
            auto undoCount = get_shared_int(SV_UNDO_COUNT);
            auto redoCount = get_shared_int(SV_REDO_COUNT);

            if (ImGui::Button("Undo") && undoCount > 0) send_command("undo");
            ImGui::SameLine();
            if (ImGui::Button("Redo") && redoCount > 0) send_command("redo");
            ImGui::SameLine();
            if (ImGui::Button("Reset")) send_command("reset");
            ImGui::SameLine();
            if (ImGui::Button("Deselect")) send_command("deselect");

            ImGui::Text("Undo: %d | Redo: %d", undoCount, redoCount);

            // Coordinate input
            ImGui::Separator();
            if (ImGui::Button("Numeric Input"))
                showCoordInput = !showCoordInput;

            if (showCoordInput)
            {
                ImGui::InputText("##coord", coordInputBuf, sizeof(coordInputBuf));
                ImGui::SameLine();
                if (ImGui::Button("Apply"))
                {
                    send_command(std::string("input:") + coordInputBuf);
                    coordInputBuf[0] = '\0';
                    showCoordInput = false;
                }
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No object selected");
        }

        // Actor browser
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Actor Browser"))
        {
            ImGui::InputText("Class", classFilterBuf, sizeof(classFilterBuf));
            ImGui::SameLine();
            if (ImGui::Button("Search"))
            {
                send_command(std::string("browse:") + classFilterBuf);
            }
            ImGui::Text("(Results appear in UE4SS console)");
            ImGui::Text("Use TOOL.selectFromBrowse(N) in console");
        }

        // Status
        if (!status.empty())
        {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", status.c_str());
        }

        // Help
        if (ImGui::CollapsingHeader("Keybindings"))
        {
            ImGui::Text("F2          Toggle TOOL");
            ImGui::Text("G/R/S/E     Move/Rotate/Scale/Elevate");
            ImGui::Text("X/Y/Z       Constrain to axis");
            ImGui::Text("Tab         Cycle axis");
            ImGui::Text("Arrows      Nudge object");
            ImGui::Text("PgUp/PgDn   Elevate");
            ImGui::Text("Ctrl+Z/Y    Undo/Redo");
            ImGui::Text("Delete      Reset transform");
            ImGui::Text("Escape      Deselect");
        }
    }
};

#define MY_AWESOME_MOD_API __declspec(dllexport)
extern "C"
{
    MY_AWESOME_MOD_API RC::CppUserModBase* start_mod()
    {
        return new InZoiTOOL_GUI();
    }

    MY_AWESOME_MOD_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
