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

bool obs_module_load(void)
{
	// Create a media source for displaying video
	obs_data_t *settings = obs_data_create();
	if (!settings) {
		blog(LOG_ERROR, "Failed to create settings data");
		return false;
	}

	obs_data_set_string(settings, "local_file", "/path/to/video.mp4"); // Replace with your file path

	media_source = obs_source_create("ffmpeg_source", "Video Source", settings, NULL);
	obs_data_release(settings);

	if (!media_source) {
		blog(LOG_ERROR, "Failed to create media source");
		return false;
	}

	// Get the current scene from the frontend
	obs_scene_t *scene = obs_scene_from_source(obs_frontend_get_current_scene());
	if (!scene) {
		blog(LOG_ERROR, "Failed to get current scene");
		return false;
	}

	// Add the media source to the scene
	obs_scene_add(scene, media_source);

	// Start playing the media source
	obs_source_media_play_pause(media_source, false); // Start playing

	obs_register_source(&playlist_source_template);
	return true;
}

void obs_module_unload(void)
{
	if (media_source) {
		obs_source_release(media_source);
	}
	obs_log(LOG_INFO, "plugin unloaded");
}