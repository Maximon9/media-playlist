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

#include "../include/sources/playlist-source.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

obs_source_t *media_source = NULL;

// Callback to handle scene initialization after OBS is fully loaded
void on_scene_initialized(enum obs_frontend_event event, void *private_data)
{
	if (event != OBS_FRONTEND_EVENT_SCENE_CHANGED) {
		return;
	}

	// Get the current scene from the source
	obs_source_t *scene_source = obs_frontend_get_current_scene();

	obs_scene_t *scene = obs_scene_from_source(scene_source);

	if (!scene) {
		obs_log(LOG_ERROR, "Failed to get current scene after initialization");
		return;
	}
	const char *scene_name = obs_source_get_name(scene_source);
	obs_log(LOG_INFO, "PRINTING Scene Name %s %d %d", scene_name);

	if (media_source == NULL && strcmp(scene_name, "Starting Soon") == 0) {
		obs_data_t *settings = obs_data_create();
		if (!settings) {
			obs_log(LOG_ERROR, "Failed to create settings data");
			return;
		}

		// Set up the video path for FFmpeg source
		const char *video_path =
			"C:/Users/aamax/OneDrive/Documents/OBSSceneVids/Start Of Purple Pink Orange Arcade Pixel Just Chatting Twitch Screen.mp4"; // Replace with your actual video path
		obs_log(LOG_INFO, "Setting video source path: %s", video_path);
		obs_data_set_string(settings, "local_file", video_path);

		// Try creating the media source
		media_source = obs_source_create_private("ffmpeg_source", "Video Source", settings);

		obs_data_release(settings);

		// obs_source_add_audio_capture_callback(media_source, playlist_audio_callback, NULL);

		if (!media_source) {
			obs_log(LOG_ERROR, "Failed to create media source");
			return;
		}
		obs_log(LOG_INFO, "Scene is initialized. Adding media source...");

		// Add media source to the scene
		obs_scene_add(scene, media_source);
		obs_log(LOG_INFO, "Media source added to scene");
	}

	// Start media playback
	// if (media_source != NULL) {
	// 	obs_source_media_play_pause(media_source, false); // Play the media
	// 	obs_log(LOG_INFO, "Media playback started");
	// }
}

// MediaFileDataArray test_array;

// Called when the plugin is loaded
bool obs_module_load(void)
{
	obs_log(LOG_INFO, "Playlist Plugin Loaded");
	// da_init(test_array);

	// push_media_back(
	// 	&test_array,
	// 	"C:/Users/aamax/OneDrive/Documents/OBSSceneVids/Start Of Purple Pink Orange Arcade Pixel Just Chatting Twitch Screen.mp4");

	// obs_log(LOG_INFO, "NERD");
	// obs_log_media_array(LOG_INFO, &test_array, 90, "    ");

	// obs_register_source(&playlist_source_template);

	// obs_frontend_add_event_callback(on_scene_initialized, NULL);

	// Initialize two MediaFileData arrays
	MediaFileDataArray source, destination;

	init_media_array(&source, 5);
	init_media_array(&destination, 5);

	// Add some media to the source array
	push_media_back(&source, "path1/file1.mp4");
	push_media_back(&source, "path2/file2.mp4");
	push_media_back(&source, "path3/file3.mp4");

	// Add some media to the destination array
	push_media_back(&destination, "pathA/fileA.mp4");
	push_media_back(&destination, "pathB/fileB.mp4");

	// Now, move the source array into the destination array
	move_media_array(&destination, &source);

	// The source array is now empty and its data is transferred to the destination array

	// Print the contents of the destination array to verify
	for (size_t i = 0; i < destination.size; i++) {
		const MediaFileData *data = get_media(&destination, i);
		printf("Media %zu: %s, %s, %s, %s\n", i, data->path, data->filename, data->name, data->ext);
	}

	// Clean up memory
	free_media_array(&destination); // This will free all media and the destination array

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
	obs_log(LOG_INFO, "Playlist Plugin Unloaded");
}

#pragma endregion