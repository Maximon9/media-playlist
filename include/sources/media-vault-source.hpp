#ifndef PLAYLIST_SOURCE_H
#define PLAYLIST_SOURCE_H

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include "../include/utils/media-file-array-utils.hpp"
#include "../include/utils/utils.hpp"
#include "../include/qt-classes/multi-media-vault-queue-viewer.hpp"

#pragma region Media Functions
void media_vault_global_signal_callback(void *data, const char *signal, calldata_t *callback_data);

const char *get_current_media_input(obs_data_t *settings);

void refresh_queue_list(MediaVaultData *media_vault_context);

void media_vault_media_source_ended(void *data, const char *signal, calldata_t *callback);

void set_queue(MediaVaultContext *media_vault_context);
// void media_vault_queue(MediaVaultContext *media_vault_context);

// void media_vault_queue_restart(MediaVaultContext *media_vault_context);

void clear_any_media_playing(MediaVaultContext *media_vault_context);

void media_vault_audio_callback(void *data, obs_source_t *source, const struct audio_data *audio_data, bool muted);

bool uses_media_history_limit(MediaVaultContext *media_vault_context);

#pragma endregion

#pragma region Property Managment

obs_properties_t *make_media_vault_properties(MediaVaultData *media_vault_data);

void update_media_vault_data(MediaVaultData *media_vault_data, obs_data_t *settings);

#pragma endregion

#pragma region Playlist Main Functions

const char *media_vault_source_name(void *data);

void *media_vault_source_create(obs_data_t *settings, obs_source_t *source);

void media_vault_source_destroy(void *data);

uint32_t media_vault_source_width(void *data);

uint32_t media_vault_source_height(void *data);

void media_vault_get_defaults(obs_data_t *settings);

obs_properties_t *media_vault_get_properties(void *data);

void media_vault_update(void *data, obs_data_t *settings);

void media_vault_activate(void *data);

void media_vault_deactivate(void *data);

void media_vault_video_tick(void *data, float seconds);

void media_vault_video_render(void *data, gs_effect_t *effect);

bool media_vault_audio_render(void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio_output, uint32_t mixers,
			      size_t channels, size_t sample_rate);

// obs_properties_t *media_vault_get_properties2(void *data, void *type_data);

void media_vault_enum_active_sources(void *data, obs_source_enum_proc_t enum_callback, void *param);

void media_vault_save(void *data, obs_data_t *settings);

void media_vault_load(void *data, obs_data_t *settings);

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

// static struct obs_source_info media_vault_source_template;

// #pragma endregion

#endif // PLAYLIST_SOURCE_H