#ifndef PLAYLIST_SOURCE_H
#define PLAYLIST_SOURCE_H

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include "../include/utils/utils.h"
#include "../include/utils/enum-utils.h"

static const char *media_filter =
	" (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm *.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";
static const char *video_filter = " (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm);;";
static const char *audio_filter = " (*.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";

struct PlaylistSource {
#pragma region Public
	obs_source_t *source;
	obs_source_t *current_media_source;
	int start_index;
	int end_index;
	enum StartBehavior playlist_start_behavior;
	enum EndBehavior playlist_end_behavior;
	int loop_index;
	bool infinite;
	int loop_count;
	MediaFileDataArray *all_media;
	MediaFileData *current_media;
	size_t current_media_index;
	bool debug;
#pragma endregion
#pragma region Private
	bool run;
#pragma endregion
};

const char *playlist_source_name(void *data);

void playlist_on_scene_switch(enum obs_frontend_event event, void *private_data);

void *playlist_source_create(obs_data_t *settings, obs_source_t *source);

void playlist_source_destroy(void *data);

uint32_t playlist_source_width(void *data);

uint32_t playlist_source_height(void *data);

void playlist_get_defaults(obs_data_t *settings);

obs_properties_t *make_playlist_properties();

obs_properties_t *playlist_get_properties(void *data);

void playlist_update(void *data, obs_data_t *settings);

void update_playlist_data(struct PlaylistSource *playlist_data, obs_data_t *settings);

void playlist_activate(void *data);

void playlist_deactivate(void *data);

void playlist_tick(void *data, float seconds);

void playlist_video_render(void *data, gs_effect_t *effect);

void media_play_pause(void *data, bool pause);

void media_restart(void *data);

void media_stop(void *data);

void media_next(void *data);

void media_previous(void *data);

int64_t media_get_duration(void *data);

int64_t media_get_time(void *data);

void media_set_time(void *data, int64_t miliseconds);

enum obs_media_state media_get_state(void *data);

static struct obs_source_info playlist_source_template = {
	.id = "media_playlist_source_codeyan",
	.type = OBS_SOURCE_TYPE_INPUT,
	.get_name = playlist_source_name,
	.create = playlist_source_create,
	.destroy = playlist_source_height,
	.get_width = playlist_source_width,
	.get_height = playlist_source_height,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_AUDIO | OBS_SOURCE_CONTROLLABLE_MEDIA,
	.get_defaults = playlist_get_defaults,
	.get_properties = playlist_get_properties,
	.update = playlist_update,
	.video_tick = playlist_tick,
	.video_render = playlist_video_render,
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

#endif // PLAYLIST_SOURCE_H