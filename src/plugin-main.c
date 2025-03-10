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

#include <obs-module.h>
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

static uint32_t sequential_media_source_width(void *data)
{
	return 1920;
}

static uint32_t sequential_media_source_height(void *data)
{
	return 1080;
}

static void *sequential_media_source_create(obs_data_t *settings, obs_source_t *source)
{
	obs_log(LOG_INFO, "We made it");
	return NULL;
}

static void sequential_media_source_destroy(void *data)
{
	obs_data_release(data);
}

static const char *sequential_media_source_name(void *data)
{
	return "Sequential Media Source"; // This should match the filename (without extension) in data/
}

struct obs_source_info sequential_media_source_info = {
	.id = "sequential_media_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO,
	.get_name = sequential_media_source_name,
	.get_width = sequential_media_source_width,
	.get_height = sequential_media_source_height,
	.create = sequential_media_source_create,
	.destroy = sequential_media_source_destroy,
	.icon_type = OBS_ICON_TYPE_MEDIA,
};

bool obs_module_load(void)
{

	// 	.id = "sequential_media_source",
	// 	.type = OBS_SOURCE_TYPE_INPUT,
	// 	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO,
	// 	.get_name = []() { return "Sequential Media Source"; },
	// 	.create = sequential_media_source_create,
	// 	.destroy = sequential_media_source_destroy,
	// 	.get_icon = sequential_media_source_icon, // Assign the icon function
	// };

	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	obs_register_source(&sequential_media_source_info);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
