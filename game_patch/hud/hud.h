#pragma once

#include "../rf/input.h"
#include <variant>
#include <vector>

enum class ChatMenuType : int
{
    None,
    Comms,
    Taunts,
    Commands,
    Spectate
};

void hud_apply_patches();
int hud_get_small_font();
int hud_get_default_font();
int hud_get_large_font();
bool hud_weapons_is_double_ammo();
void draw_hud_vote_notification(std::string vote_type);
void remove_hud_vote_notification();
void stop_draw_respawn_timer_notification();
void draw_respawn_timer_notification(bool can_respawn, bool force_respawn, int spawn_delay);
void draw_hud_ready_notification(bool draw);
void set_local_pre_match_active(bool set_active);
void multi_hud_level_init();
void toggle_chat_menu(ChatMenuType state);
bool get_chat_menu_is_active();
void hud_render_draw_chat_menu();
void chat_menu_action_handler(rf::Key key);
void build_local_player_spectators_strings();

inline struct RemoteServerCfgPopup {
public:
    void reset();
    void add_content(std::string_view content);
    bool is_active();
    void toggle();
    void render();

    void set_cfg_changed() {
        m_cfg_changed = true;
    }

private:
    void add_line(std::string_view line);

    using Line = std::variant<std::string, std::pair<std::string, std::string>>;
    std::vector<Line> m_lines{};
    std::string m_partial_line{};
    bool m_cfg_changed = false;
    bool m_is_active = false;
    struct {
        float current = 0.f;
        float target = 0.f;
        float velocity = 0.f;
    } m_scroll{};
} g_remote_server_cfg_popup{};
