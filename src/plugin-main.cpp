/*
Sequential Media Source
Copyright (C) 2025 Maximon9

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#pragma region Main

#include "../include/sources/media-vault-source.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

/* void test_callback_stuff(enum obs_frontend_event event, void *private_data)
{
	obs_log(LOG_INFO, "Test Event: %d", event);
} */

// typedef struct BasicBruv {
// 	int not_ptr;
// 	int *ptr;
// } BasicBruv;

// typedef std::deque<BasicBruv> BasicBruvArray;

typedef struct test_str_struct {
	std::string str;
} test_str_struct;

// Called when the plugin is loaded
bool obs_module_load(void)
{
	QWidget *obs_main_window = (QWidget *)obs_frontend_get_main_window();
	multi_media_vault_queue_viewer = new MultiMediaVaultQueueViewer(obs_main_window);

	if (obs_frontend_add_dock_by_id("multi_vault_queue_viewer_maximon9", "Multi Media Vault Queue Viewer",
					multi_media_vault_queue_viewer) == false) {
		delete multi_media_vault_queue_viewer;
	}

	struct obs_source_info media_vault_source_template{};

	media_vault_source_template.id = "media_vault_maximon9",
	media_vault_source_template.type = OBS_SOURCE_TYPE_INPUT;
	media_vault_source_template.get_name = media_vault_source_name;
	media_vault_source_template.create = media_vault_source_create;
	media_vault_source_template.destroy = media_vault_source_destroy;
	media_vault_source_template.get_width = media_vault_source_width;
	media_vault_source_template.get_height = media_vault_source_height;
	media_vault_source_template.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_AUDIO |
						   OBS_SOURCE_CONTROLLABLE_MEDIA;
	media_vault_source_template.get_defaults = media_vault_get_defaults;
	media_vault_source_template.get_properties = media_vault_get_properties;
	media_vault_source_template.update = media_vault_update;
	media_vault_source_template.activate = media_vault_activate;
	media_vault_source_template.deactivate = media_vault_deactivate;
	media_vault_source_template.video_tick = media_vault_video_tick;
	media_vault_source_template.video_render = media_vault_video_render;
	media_vault_source_template.audio_render = media_vault_audio_render;
	// media_vault_source_template.get_properties2 = media_vault_get_properties2;
	media_vault_source_template.enum_active_sources = media_vault_enum_active_sources;
	media_vault_source_template.save = media_vault_save;
	media_vault_source_template.load = media_vault_load;
	media_vault_source_template.icon_type = OBS_ICON_TYPE_MEDIA;
	media_vault_source_template.media_play_pause = media_play_pause;
	media_vault_source_template.media_restart = media_restart;
	media_vault_source_template.media_stop = media_stop;
	media_vault_source_template.media_next = media_next;
	media_vault_source_template.media_previous = media_previous;
	media_vault_source_template.media_get_duration = media_get_duration;
	media_vault_source_template.media_get_time = media_get_time;
	media_vault_source_template.media_set_time = media_set_time;
	media_vault_source_template.media_get_state = media_get_state;

	obs_log(LOG_INFO, "%s plugin loaded successfully (version %s)", PLUGIN_NAME, PLUGIN_VERSION);
	// da_init(test_array);

	// obs_log(LOG_INFO, "NERD");
	// obs_log_media_array(LOG_INFO, &test_array, 90, "    ");

	obs_register_source(&media_vault_source_template);

	// obs_frontend_add_event_callback(on_scene_initialized, NULL);

	// Initialize two MediaContext arrays

	return true;
}

// Called when the plugin is unloaded
void obs_module_unload(void)
{
	// obs_frontend_remove_event_callback(on_scene_initialized, NULL);

	// if (media_source) {
	// 	obs_source_release(media_source);
	// 	media_source = NULL;
	// 	obs_log(LOG_INFO, "Media source released successfully");
	// }
	// da_free(test_array);
	// if (multi_media_vault_queue_viewer != nullptr) {
	// 	delete multi_media_vault_queue_viewer;
	// 	multi_media_vault_queue_viewer = nullptr;
	// }
	obs_log(LOG_INFO, "%s plugin unloaded %s", PLUGIN_NAME, PLUGIN_VERSION);
}

#pragma endregion