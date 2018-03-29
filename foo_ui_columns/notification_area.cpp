#include "stdafx.h"

#include "notification_area.h"
#include "main_window.h"

void update_systray(bool balloon, int btitle, bool force_balloon)
{
    if (g_icon_created) {
        metadb_handle_ptr track;
        static_api_ptr_t<play_control> play_api;
        play_api->get_now_playing(track);
        pfc::string8 sys, title;

        if (track.is_valid()) {
            service_ptr_t<titleformat_object> to_systray;
            static_api_ptr_t<titleformat_compiler>()->compile_safe(
                to_systray, main_window::config_notification_icon_script.get());
            play_api->playback_format_title_ex(
                track, nullptr, title, to_systray, nullptr, play_control::display_level_titles);

            track.release();

        } else {
            title = "foobar2000"; // core_version_info::g_get_version_string();
        }

        uFixAmpersandChars(title, sys);

        if (balloon && (cfg_balloon || force_balloon)) {
            uShellNotifyIconEx(NIM_MODIFY, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, sys, "", "");
            uShellNotifyIconEx(NIM_MODIFY, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, sys,
                (btitle == 0 ? "Now playing:" : (btitle == 1 ? "Unpaused:" : "Paused:")), title);
        } else
            uShellNotifyIcon(NIM_MODIFY, g_main_window, 1, MSG_NOTICATION_ICON, g_icon, sys);
    }
}

void destroy_systray_icon()
{
    if (g_icon_created) {
        uShellNotifyIcon(NIM_DELETE, g_main_window, 1, MSG_NOTICATION_ICON, nullptr, nullptr);
        g_icon_created = false;
    }
}

void on_show_notification_area_icon_change()
{
    if (!g_main_window)
        return;

    const auto is_iconic = IsIconic(g_main_window) != 0;
    const auto close_to_icon = cui::config::advbool_close_to_notification_icon.get();
    if (cfg_show_systray && !g_icon_created) {
        create_systray_icon();
    } else if (!cfg_show_systray && g_icon_created && (!is_iconic || !(cfg_minimise_to_tray || close_to_icon))) {
        destroy_systray_icon();
        if (is_iconic)
            standard_commands::main_activate();
    }
    if (g_status)
        update_systray();
}

void create_systray_icon()
{
    uShellNotifyIcon(g_icon_created ? NIM_MODIFY : NIM_ADD, g_main_window, 1, MSG_NOTICATION_ICON, g_icon,
        "foobar2000" /*core_version_info::g_get_version_string()*/);
    /* There was some misbehaviour with the newer messages. So we don't use them. */
    //    if (!g_icon_created)
    //        uih::shell_notify_icon(NIM_SETVERSION, g_main_window, 1, NOTIFYICON_VERSION, MSG_NOTICATION_ICON, g_icon,
    //        "foobar2000"/*core_version_info::g_get_version_string()*/);
    g_icon_created = true;
}

void create_icon_handle()
{
    const unsigned cx = GetSystemMetrics(SM_CXSMICON);
    const unsigned cy = GetSystemMetrics(SM_CYSMICON);
    if (g_icon) {
        DestroyIcon(g_icon);
        g_icon = nullptr;
    }
    if (cfg_custom_icon)
        g_icon
            = (HICON)uLoadImage(core_api::get_my_instance(), cfg_tray_icon_path, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
    if (!g_icon)
        g_icon = static_api_ptr_t<ui_control>()->load_main_icon(cx, cy);
}