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
// #include <obs-frontend-api.h>
// #include <obs-module.h>
// #include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

// void on_scene_switch(enum obs_frontend_event event, void *private_data)
// {
// 	if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED) {
// 		// Example: Get the current scene and log its name
// 		obs_source_t *scene = obs_frontend_get_current_scene();
// 		if (scene) {
// 			const char *scene_name = obs_source_get_name(scene);
// 			blog(LOG_INFO, "Current scene: %s", scene_name);
// 		}
// 	}
// }

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	obs_register_source(&playlist_source_template);
	// obs_frontend_add_event_callback(on_scene_switch, NULL);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
	// obs_frontend_remove_event_callback(on_scene_switch, NULL);
}