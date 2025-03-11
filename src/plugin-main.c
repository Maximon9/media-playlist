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
#include <obs-module.h>
#include <util/threading.h>
#include <util/platform.h>
#include <util/darray.h>
#include <util/dstr.h>
#include <util/deque.h>
#include <plugin-support.h>
#include "playlist.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

static uint32_t playlist_source_width(void *data)
{
	return 1920;
}

static uint32_t playlist_source_height(void *data)
{
	return 1080;
}

struct playlist_source {
	obs_source_t *source;
	obs_source_t *current_media_source;

	bool shuffle;
	bool loop;
	bool paused;
	bool user_stopped;
	bool use_hw_decoding;
	bool close_when_inactive;
	pthread_mutex_t mutex;
	DARRAY(struct media_file_data) files;
	struct media_file_data *current_media; // only for file/folder in the list
	struct media_file_data *actual_media;  // for both files and folder items
	size_t current_media_index;
	char *current_media_filename; // only used with folder_items
	// to know if current_folder_item_index will be used, check if current file is a folder
	size_t current_folder_item_index;
	long long speed;
	bool first_update;

	obs_hotkey_id play_pause_hotkey;
	obs_hotkey_id restart_hotkey;
	obs_hotkey_id stop_hotkey;
	obs_hotkey_id next_hotkey;
	obs_hotkey_id prev_hotkey;

	enum obs_media_state state;
	enum visibility_behavior visibility_behavior;
	enum restart_behavior restart_behavior;

	struct deque audio_data[MAX_AUDIO_CHANNELS];
	struct deque audio_frames;
	struct deque audio_timestamps;
	size_t num_channels;
	pthread_mutex_t audio_mutex;
};

static void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);
	obs_log(LOG_INFO, "We made it");

	struct playlist_source *mps = bzalloc(sizeof(*mps));

	mps->first_update = true;
	mps->source = source;

	/* Internal media source */
	obs_data_t *media_source_data = obs_data_create();
	obs_data_release(media_source_data);

	return mps;
}

static void playlist_source_destroy(void *data)
{
	obs_data_release(data);
}

static const char *playlist_source_name(void *data)
{
	return "Playlist"; // This should match the filename (without extension) in data/
}

struct obs_source_info playlist_source_info = {
	.id = "playlist",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO,
	.get_name = playlist_source_name,
	.get_width = playlist_source_width,
	.get_height = playlist_source_height,
	.create = *playlist_source_create,
	.destroy = playlist_source_destroy,
	.icon_type = OBS_ICON_TYPE_MEDIA,
};

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
