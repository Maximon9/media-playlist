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

#include "../include/sources/playlist-source.h"
#include <obs-frontend-api.h>
// #include <obs-module.h>
// #include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

obs_source_t *media_source = NULL;

// Callback to handle scene initialization after OBS is fully loaded
void on_scene_initialized(enum obs_frontend_event event, void *private_data)
{
	if (event != OBS_FRONTEND_EVENT_SCENE_CHANGED) {
		return;
	}

	obs_log(LOG_INFO, "Scene is initialized. Adding media source...");

	// Get the current scene from the source
	obs_scene_t *scene = obs_scene_from_source(obs_frontend_get_current_scene());
	if (!scene) {
		obs_log(LOG_ERROR, "Failed to get current scene after initialization");
		return;
	}

	// Add media source to the scene
	obs_scene_add(scene, media_source);
	obs_log(LOG_INFO, "Media source added to scene");

	// Start media playback
	obs_source_media_play_pause(media_source, false); // Play the media
	obs_log(LOG_INFO, "Media playback started");
}

// Called when the plugin is loaded
bool obs_module_load(void)
{
	obs_log(LOG_INFO, "Plugin loading...");

	obs_data_t *settings = obs_data_create();
	if (!settings) {
		obs_log(LOG_ERROR, "Failed to create settings data");
		return false;
	}

	// Set up the video path for FFmpeg source
	const char *video_path =
		"C:/Users/aamax/OneDrive/Documents/OBSSceneVids/Start Of Purple Pink Orange Arcade Pixel Just Chatting Twitch Screen.mp4"; // Replace with your actual video path
	obs_log(LOG_INFO, "Setting video source path: %s", video_path);
	obs_data_set_string(settings, "local_file", video_path);

	// Try creating the media source
	media_source = obs_source_create("ffmpeg_source", "Video Source", settings, NULL);
	obs_data_release(settings);

	if (!media_source) {
		obs_log(LOG_ERROR, "Failed to create media source");
		return false;
	}

	// Register the event callback for when the scene is initialized
	obs_frontend_add_event_callback(on_scene_initialized, NULL);
	obs_log(LOG_INFO, "Event callback for scene initialization registered");

	return true;
}

// Called when the plugin is unloaded
void obs_module_unload(void)
{
	obs_log(LOG_INFO, "Unloading plugin...");

	// Remove the event callback
	obs_frontend_remove_event_callback(on_scene_initialized, NULL);

	// Clean up media source
	if (media_source) {
		obs_source_release(media_source);
		media_source = NULL;
		obs_log(LOG_INFO, "Media source released successfully");
	}

	obs_log(LOG_INFO, "Plugin unloaded");
}