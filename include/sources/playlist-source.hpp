#ifndef PLAYLIST_SOURCE_H
#define PLAYLIST_SOURCE_H

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include "../include/qt-classes/custom-properties.hpp"

#pragma region Media Functions
void playlist_global_signal_callback(void *data, const char *signal, calldata_t *callback_data);

const char *get_current_media_input(obs_data_t *settings);

void refresh_queue_list(PlaylistSource *playlist_data);

void playlist_media_source_ended(void *data, calldata_t *callback);

void playlist_queue(PlaylistSource *playlist_data);

void playlist_queue_restart(PlaylistSource *playlist_data);

void clear_any_media_playing(PlaylistSource *playlist_data);

void playlist_audio_callback(void *data, obs_source_t *source, const struct audio_data *audio_data, bool muted);

bool uses_song_history_limit(PlaylistSource *playlist_data);

#pragma endregion

#pragma region Property Managment

bool playlist_queue_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings);

obs_properties_t *update_playlist_properties();

void update_playlist_data(PlaylistSource *playlist_data, obs_data_t *settings);

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

// obs_properties_t *playlist_get_properties2(void *data, void *type_data);

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

// #pragma region Playlist Template

// static struct obs_source_info playlist_source_template;

// #pragma endregion

#endif // PLAYLIST_SOURCE_H