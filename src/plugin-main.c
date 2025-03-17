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
// #include <obs-module.h>
// #include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

// void _on_scene_switch(enum obs_frontend_event event, void *private_data) {}

bool obs_module_load(void)
{
	obs_register_source(&playlist_source_template);
	// obs_frontend_add_event_callback(_on_scene_switch, NULL);
	return true;
}

void obs_module_unload(void)
{
	// obs_frontend_remove_event_callback(_on_scene_switch, NULL);
	obs_log(LOG_INFO, "plugin unloaded");
}