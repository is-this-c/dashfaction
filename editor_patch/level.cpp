#include <patch_common/FunHook.h>
#include <patch_common/CodeInjection.h>
#include <patch_common/AsmWriter.h>
#include <patch_common/MemUtils.h>
#include <xlog/xlog.h>
#include "level.h"
#include "vtypes.h"
#include "mfc_types.h"
#include "resources.h"

// add AlpineLevelProperties chunk after stock game chunks when creating a new level
CodeInjection CDedLevel_construct_patch{
    0x004181B8,
    [](auto& regs) {
        std::byte* level = regs.esi;
        new (&level[stock_cdedlevel_size]) AlpineLevelProperties();
    },
};

// load default AlpineLevelProperties values
CodeInjection CDedLevel_LoadLevel_patch1{
    0x0042F136,
    []() {
        CDedLevel::Get()->GetAlpineLevelProperties().LoadDefaults();
    },
};

// load AlpineLevelProperties chunk from rfl file
CodeInjection CDedLevel_LoadLevel_patch2{
    0x0042F2D4,
    [](auto& regs) {
        auto& file = *static_cast<rf::File*>(regs.esi);

        // Alpine level properties chunk was introduced in rfl v302, no point looking for it before that
        if (file.check_version(302)) {
            auto& level = *static_cast<CDedLevel*>(regs.ebp);
            int chunk_id = regs.edi;
            std::size_t chunk_size = regs.ebx;
            if (chunk_id == alpine_props_chunk_id) {
                auto& alpine_level_props = level.GetAlpineLevelProperties();
                alpine_level_props.Deserialize(file, chunk_size);
                regs.eip = 0x0043090C;
            }
        }
    },
};

// save AlpineLevelProperties when saving rfl file
CodeInjection CDedLevel_SaveLevel_patch{
    0x00430CBD,
    [](auto& regs) {
        auto& level = *static_cast<CDedLevel*>(regs.edi);
        auto& file = *static_cast<rf::File*>(regs.esi);
        auto start_pos = level.BeginRflSection(file, alpine_props_chunk_id);
        auto& alpine_level_props = level.GetAlpineLevelProperties();
        alpine_level_props.Serialize(file);
        level.EndRflSection(file, start_pos);
    },
};

// load AlpineLevelProperties settings when opening level properties dialog
CodeInjection CLevelDialog_OnInitDialog_patch{
    0x004676C0,
    [](auto& regs) {
        HWND hdlg = WndToHandle(regs.esi);
        auto& alpine_level_props = CDedLevel::Get()->GetAlpineLevelProperties();
        CheckDlgButton(hdlg, IDC_LEGACY_CYCLIC_TIMERS, alpine_level_props.legacy_cyclic_timers ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hdlg, IDC_LEGACY_MOVERS, alpine_level_props.legacy_movers ? BST_CHECKED : BST_UNCHECKED);
    },
};

// save AlpineLevelProperties settings when closing level properties dialog
CodeInjection CLevelDialog_OnOK_patch{
    0x00468470,
    [](auto& regs) {
        HWND hdlg = WndToHandle(regs.ecx);
        auto& alpine_level_props = CDedLevel::Get()->GetAlpineLevelProperties();
        alpine_level_props.legacy_cyclic_timers = IsDlgButtonChecked(hdlg, IDC_LEGACY_CYCLIC_TIMERS) == BST_CHECKED;
        alpine_level_props.legacy_movers = IsDlgButtonChecked(hdlg, IDC_LEGACY_MOVERS) == BST_CHECKED;
    },
};

void ApplyLevelPatches()
{
    // include space for default AlpineLevelProperties chunk in newly created rfls
    write_mem<std::uint32_t>(0x0041C906 + 1, 0x668 + sizeof(AlpineLevelProperties));

    // handle AlpineLevelProperties chunk
    CDedLevel_construct_patch.install();
    CDedLevel_LoadLevel_patch1.install();
    CDedLevel_LoadLevel_patch2.install();
    CDedLevel_SaveLevel_patch.install();
    CLevelDialog_OnInitDialog_patch.install();
    CLevelDialog_OnOK_patch.install();

    // Avoid clamping lightmaps when loading rfl files
    AsmWriter{0x004A5D6A}.jmp(0x004A5D6E);

    // Default level ambient light and fog color to flat black
    constexpr std::uint8_t default_ambient_light = 0;
    constexpr std::uint8_t default_fog = 0;
    write_mem<std::uint8_t>(0x0041CABD + 1, default_ambient_light);
    write_mem<std::uint8_t>(0x0041CABF + 1, default_ambient_light);
    write_mem<std::uint8_t>(0x0041CAC1 + 1, default_ambient_light);
    write_mem<std::uint8_t>(0x0041CAD3 + 1, default_ambient_light);
    write_mem<std::uint8_t>(0x0041CAD5 + 1, default_ambient_light);
    write_mem<std::uint8_t>(0x0041CAD7 + 1, default_ambient_light);
    write_mem<std::uint8_t>(0x0041CB07 + 1, default_fog);
    write_mem<std::uint8_t>(0x0041CB09 + 1, default_fog);
    write_mem<std::uint8_t>(0x0041CB0B + 1, default_fog);
}
