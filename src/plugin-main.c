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

// #include <obs-module.h>
#include "playlist_source.h"
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

// static uint32_t playlist_source_width(void *data)
// {
// 	return 1920;
// }

// static uint32_t playlist_source_height(void *data)
// {
// 	return 1080;
// }

// struct playlist_source *playlist = {0};
// static void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
// {
// 	UNUSED_PARAMETER(settings);
// 	obs_log(LOG_INFO, "We made it");

// 	return playlist;
// }

// static void playlist_source_destroy(void *data)
// {
// 	obs_data_release(data);
// }

// static const char *playlist_source_name(void *data)
// {
// 	return "Playlist"; // This should match the filename (without extension) in data/
// }

// static obs_properties_t *playlist_get_properties(void *data)
// {
// 	obs_properties_t *props = obs_properties_create();
// 	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
// 					 media_filter, "");
// 	obs_properties_add_bool(props, "loop", "Loop");
// 	return props;
// }

// static void playlist_get_defaults(obs_data_t *settings)
// {
// 	obs_data_set_default_bool(settings, "loop", false);
// }

// static struct obs_source_info playlist_source_info = {
// 	.id = "playlist",
// 	.type = OBS_SOURCE_TYPE_INPUT,
// 	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO,
// 	.get_name = *playlist_source_name,
// 	.get_width = *playlist_source_width,
// 	.get_height = *playlist_source_height,
// 	.create = *playlist_source_create,
// 	.destroy = *playlist_source_destroy,
// 	.icon_type = OBS_ICON_TYPE_MEDIA,
// 	.get_properties = *playlist_get_properties,
// 	.get_defaults = *playlist_get_defaults,
// };

bool obs_module_load(void)
{

	// 	.id = "sequential_media_source",
	// 	.type = OBS_SOURCE_TYPE_INPUT,
	// 	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO,
	// 	.get_name = []() { return "Sequential Media Source"; },
	// 	.create = playlist_source_create,
	// 	.destroy = playlist_source_destroy,
	// 	.get_icon = sequential_media_source_icon, // Assign the icon function
	// };

	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	obs_register_source(&playlist_source_info);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
