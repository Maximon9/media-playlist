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
	// test_str_struct test_string{};
	// test_string.str = "wassup";

	// test_str_struct test_string_1{};
	// test_string_1.str = "boyo";

	// obs_log(LOG_INFO, "Testing: %s, %s", test_string.str, test_string_1.str);

	// test_string = test_string_1;

	// obs_log(LOG_INFO, "Testing: %s, %s", test_string.str, test_string_1.str);

	// test_string.str = "ok bruh";

	// obs_log(LOG_INFO, "Testing: %s, %s", test_string.str, test_string_1.str);

	// test_str_struct test_string_3 {};
	// test_string_3.std = "boyo";

	// BasicBruvArray basic_bruv_array_1{};

	// BasicBruv basic_bruv_1{};

	// int this_int_1 = 9;

	// basic_bruv_1.not_ptr = 8;
	// basic_bruv_1.ptr = &this_int_1;

	// basic_bruv_array_1.push_back(basic_bruv_1);

	// BasicBruvArray basic_bruv_array_2{};

	// BasicBruv basic_bruv_2{};

	// int this_int_2 = 19;

	// basic_bruv_2.not_ptr = 18;
	// basic_bruv_2.ptr = &this_int_2;

	// basic_bruv_array_2.push_back(basic_bruv_2);

	// obs_log(LOG_INFO, "Basic Bruv 1: %d, %d", *basic_bruv_array_1[0].ptr, basic_bruv_array_1[0].not_ptr);
	// obs_log(LOG_INFO, "Basic Bruv 2: %d, %d", *basic_bruv_array_2[0].ptr, basic_bruv_array_2[0].not_ptr);

	// std::swap(basic_bruv_array_1[0], basic_bruv_array_2[0]);

	// obs_log(LOG_INFO, "Basic Bruv 1: %d, %d", *basic_bruv_array_1[0].ptr, basic_bruv_array_1[0].not_ptr);
	// obs_log(LOG_INFO, "Basic Bruv 2: %d, %d", *basic_bruv_array_2[0].ptr, basic_bruv_array_2[0].not_ptr);

	// obs_log(LOG_INFO, "Loading Playlists");

	QWidget *obs_main_window = (QWidget *)obs_frontend_get_main_window();
	multi_playlist_queue_viewer = new MultiPlaylistQueueViewer(obs_main_window);

	if (obs_frontend_add_dock_by_id("playlist_queue_viewer_maximon9", "Playlist Queue Viewer",
					multi_playlist_queue_viewer) == false) {
		delete multi_playlist_queue_viewer;
	}

	struct obs_source_info playlist_source_template{};

	playlist_source_template.id = "media_playlist_code_maximon9",
	playlist_source_template.type = OBS_SOURCE_TYPE_INPUT;
	playlist_source_template.get_name = playlist_source_name;
	playlist_source_template.create = playlist_source_create;
	playlist_source_template.destroy = playlist_source_destroy;
	playlist_source_template.get_width = playlist_source_width;
	playlist_source_template.get_height = playlist_source_height;
	playlist_source_template.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_AUDIO |
						OBS_SOURCE_CONTROLLABLE_MEDIA;
	playlist_source_template.get_defaults = playlist_get_defaults;
	playlist_source_template.get_properties = playlist_get_properties;
	playlist_source_template.update = playlist_update;
	playlist_source_template.activate = playlist_activate;
	playlist_source_template.deactivate = playlist_deactivate;
	playlist_source_template.video_tick = playlist_video_tick;
	playlist_source_template.video_render = playlist_video_render;
	playlist_source_template.audio_render = playlist_audio_render;
	// playlist_source_template.get_properties2 = playlist_get_properties2;
	playlist_source_template.enum_active_sources = playlist_enum_active_sources;
	playlist_source_template.save = playlist_save;
	playlist_source_template.load = playlist_load;
	playlist_source_template.icon_type = OBS_ICON_TYPE_MEDIA;
	playlist_source_template.media_play_pause = media_play_pause;
	playlist_source_template.media_restart = media_restart;
	playlist_source_template.media_stop = media_stop;
	playlist_source_template.media_next = media_next;
	playlist_source_template.media_previous = media_previous;
	playlist_source_template.media_get_duration = media_get_duration;
	playlist_source_template.media_get_time = media_get_time;
	playlist_source_template.media_set_time = media_set_time;
	playlist_source_template.media_get_state = media_get_state;

	obs_log(LOG_INFO, "%s plugin loaded successfully (version %s)", PLUGIN_NAME, PLUGIN_VERSION);
	// da_init(test_array);

	// obs_log(LOG_INFO, "NERD");
	// obs_log_media_array(LOG_INFO, &test_array, 90, "    ");

	obs_register_source(&playlist_source_template);

	// obs_frontend_add_event_callback(on_scene_initialized, NULL);

	// Initialize two MediaData arrays

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
	// if (multi_playlist_queue_viewer != nullptr) {
	// 	delete multi_playlist_queue_viewer;
	// 	multi_playlist_queue_viewer = nullptr;
	// }
	obs_log(LOG_INFO, "%s plugin unloaded %s", PLUGIN_NAME, PLUGIN_VERSION);
}

#pragma endregion