#ifndef PLAYLIST_SOURCE_H
#define PLAYLIST_SOURCE_H

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/threading.h>
// #include <util/platform.h>
// #include <util/darray.h>
// #include <util/dstr.h>
#include <util/deque.h>
#include <plugin-support.h>
#include "../include/utils/utils.h"
#include "../include/utils/enum-utils.h"

#pragma region Custom Types

struct PlaylistSource {
#pragma region Public
	bool shuffle_queue;
	obs_source_t *source;
	obs_source_t *media_source;
	obs_data_t *media_source_settings;
	int start_index;
	int end_index;
	enum StartBehavior playlist_start_behavior;
	enum EndBehavior playlist_end_behavior;
	int loop_index;
	bool infinite;
	int loop_count;
	enum LoopEndBehavior loop_end_behavior;
	bool rest_playlist_after_last_loop;
	MediaFileDataArray all_media;
	size_t current_media_index;
	int song_history_limit;
	int queue_list_size;
	bool debug;
#pragma endregion
#pragma region Private
	MediaFileDataArray queue;
	MediaFileDataArray previous_queue;
	bool run;
	bool paused;
	pthread_mutex_t mutex;
	struct deque audio_data[MAX_AUDIO_CHANNELS];
	struct deque audio_frames;
	struct deque audio_timestamps;
	size_t num_channels;
	pthread_mutex_t audio_mutex;
#pragma endregion
};

#pragma endregion

#pragma region Media Functions

#pragma region Button Callbacks

static void shuffle_list(obs_properties_t *props, obs_property_t *property, void *data);

#pragma endregion

static void playlist_queue(struct PlaylistSource *playlist_data);

static void playlist_audio_callback(void *data, obs_source_t *source, const struct audio_data *audio_data, bool muted);

#pragma endregion

#pragma region Property Managment

static obs_properties_t *make_playlist_properties();

static void update_playlist_data(struct PlaylistSource *playlist_data, obs_data_t *settings);

#pragma endregion

#pragma region Playlist Main Functions

const char *playlist_source_name(void *data);

void *playlist_source_create(obs_data_t *settings, obs_source_t *source);

void playlist_source_destroy(void *data);

uint32_t playlist_source_width(void *data);

uint32_t playlist_source_height(void *data);

void playlist_get_defaults(obs_data_t *settings);

obs_properties_t *playlist_get_properties(void *data);

void playlist_update(void *data, obs_data_t *settings);

void playlist_activate(void *data);

void playlist_deactivate(void *data);

void playlist_video_tick(void *data, float seconds);

void playlist_video_render(void *data, gs_effect_t *effect);

bool playlist_audio_render(void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio_output, uint32_t mixers,
			   size_t channels, size_t sample_rate);

void playlist_enum_active_sources(void *data, obs_source_enum_proc_t enum_callback, void *param);

void playlist_save(void *data, obs_data_t *settings);

void playlist_load(void *data, obs_data_t *settings);

void media_play_pause(void *data, bool pause);

void media_restart(void *data);

void media_stop(void *data);

void media_next(void *data);

void media_previous(void *data);

int64_t media_get_duration(void *data);

int64_t media_get_time(void *data);

void media_set_time(void *data, int64_t miliseconds);

enum obs_media_state media_get_state(void *data);

#pragma endregion

#pragma region Playlist Template

static struct obs_source_info playlist_source_template = {
	.id = "media_playlist_code_maximon9",
	.type = OBS_SOURCE_TYPE_INPUT,
	.get_name = playlist_source_name,
	.create = playlist_source_create,
	.destroy = playlist_source_destroy,
	.get_width = playlist_source_width,
	.get_height = playlist_source_height,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_AUDIO | OBS_SOURCE_CONTROLLABLE_MEDIA,
	.get_defaults = playlist_get_defaults,
	.get_properties = playlist_get_properties,
	.update = playlist_update,
	.activate = playlist_activate,
	.deactivate = playlist_deactivate,
	.video_tick = playlist_video_tick,
	.video_render = playlist_video_render,
	.audio_render = playlist_audio_render,
	.enum_active_sources = playlist_enum_active_sources,
	.save = playlist_save,
	.load = playlist_load,
	.icon_type = OBS_ICON_TYPE_MEDIA,
	.media_play_pause = media_play_pause,
	.media_restart = media_restart,
	.media_stop = media_stop,
	.media_next = media_next,
	.media_previous = media_previous,
	.media_get_duration = media_get_duration,
	.media_get_time = media_get_time,
	.media_set_time = media_set_time,
	.media_get_state = media_get_state,
};

#pragma endregion

#endif; // PLAYLIST_SOURCE_H