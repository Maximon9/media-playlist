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

#include "../include/sources/playlist-source.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

// Called when the plugin is loaded
bool obs_module_load(void)
{
	struct obs_source_info playlist_source_template{};

	playlist_source_template.id = "media_playlist_code_maximon9",
	playlist_source_template.type = OBS_SOURCE_TYPE_INPUT, playlist_source_template.get_name = playlist_source_name;
	playlist_source_template.create = playlist_source_create;
	playlist_source_template.destroy = playlist_source_destroy;
	playlist_source_template.get_width = playlist_source_width;
	playlist_source_template.get_height = playlist_source_height;
	playlist_source_template.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_AUDIO;
	OBS_SOURCE_CONTROLLABLE_MEDIA;
	playlist_source_template.get_defaults = playlist_get_defaults;
	playlist_source_template.get_properties = playlist_get_properties;
	playlist_source_template.update = playlist_update, playlist_source_template.activate = playlist_activate;
	playlist_source_template.deactivate = playlist_deactivate;
	playlist_source_template.video_tick = playlist_video_tick;
	playlist_source_template.video_render = playlist_video_render;
	playlist_source_template.audio_render = playlist_audio_render;
	playlist_source_template.enum_active_sources = playlist_enum_active_sources;
	playlist_source_template.save = playlist_save, playlist_source_template.load = playlist_load;
	playlist_source_template.icon_type = OBS_ICON_TYPE_MEDIA;
	playlist_source_template.media_play_pause = media_play_pause;
	playlist_source_template.media_restart = media_restart, playlist_source_template.media_stop = media_stop;
	playlist_source_template.media_next = media_next, playlist_source_template.media_previous = media_previous;
	playlist_source_template.media_get_duration = media_get_duration;
	playlist_source_template.media_get_time = media_get_time;
	playlist_source_template.media_set_time = media_set_time;
	playlist_source_template.media_get_state = media_get_state;

	MediaFileDataArray temp_array_1{};

	temp_array_1.push_back(create_media_file_data_from_path("Hi/there/does/this/stay.mp4", 0));

	MediaFileData *item_from_temp_array_1 = &temp_array_1[0];

	obs_log(LOG_INFO, "Temp Test Path: %s", item_from_temp_array_1->path);
	obs_log(LOG_INFO, "Temp Test File Name: %s", item_from_temp_array_1->filename);
	obs_log(LOG_INFO, "Temp Test Name: %s", item_from_temp_array_1->name);
	obs_log(LOG_INFO, "Temp Test Extension: %s", item_from_temp_array_1->ext);

	temp_array_1.erase(temp_array_1.cend());

	obs_log(LOG_INFO, "Temp Test Path: %s", item_from_temp_array_1->path);
	obs_log(LOG_INFO, "Temp Test File Name: %s", item_from_temp_array_1->filename);
	obs_log(LOG_INFO, "Temp Test Name: %s", item_from_temp_array_1->name);
	obs_log(LOG_INFO, "Temp Test Extension: %s", item_from_temp_array_1->ext);

	// MediaFileDataArray temp_array_2{};

	obs_log(LOG_INFO, "%s plugin loaded successfully (version %s)", PLUGIN_NAME, PLUGIN_VERSION);
	// da_init(test_array);

	// obs_log(LOG_INFO, "NERD");
	// obs_log_media_array(LOG_INFO, &test_array, 90, "    ");

	obs_register_source(&playlist_source_template);

	// obs_frontend_add_event_callback(on_scene_initialized, NULL);

	// Initialize two MediaFileData arrays

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
	obs_log(LOG_INFO, "%s plugin unloaded %s", PLUGIN_NAME, PLUGIN_VERSION);
}

#pragma endregion