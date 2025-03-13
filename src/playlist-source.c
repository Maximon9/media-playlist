#pragma region Main
#include "../include/playlist-source.h"

const char *playlist_source_name(void *data)
{
	return "Playlist"; // This should match the filename (without extension) in data/
}

// struct PlaylistSource playlist_data = {.playlist = NULL,
// 				       .playlist_start_behavior = RESTART,
// 				       .playlist_end_behavior = STOP};
void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
{
	// UNUSED_PARAMETER(settings);

	struct PlaylistSource *playlist = bzalloc(sizeof(*playlist));

	playlist->source = source;

	obs_data_set_bool(settings, "debug", false);
	// obs_data_set_bool(media_source_data, "log_changes", false);
	// mps->current_media_source =
	// 	obs_source_create_private("ffmpeg_source", "current_media_source", media_source_data);
	// obs_source_add_active_child(mps->source, mps->current_media_source);
	// obs_source_add_audio_capture_callback(mps->current_media_source, mps_audio_callback, mps);

	return playlist;
}

void playlist_source_destroy(void *data)
{
	obs_data_release(data);
}

uint32_t playlist_source_width(void *data)
{
	return 1920;
}

uint32_t playlist_source_height(void *data)
{
	return 1080;
}

void playlist_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_bool(settings, "loop", false);
}

obs_properties_t *playlist_get_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	obs_property_t *psb_property = obs_properties_add_list(
		props, "playlist_start_behavior", "Playlist Start Behavior", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	obs_property_list_add_int(psb_property, "Restart", RESTART);
	obs_property_list_add_int(psb_property, "Unpause", UNPAUSE);

	obs_property_t *peb_property = obs_properties_add_list(props, "playlist_end_behavior", "Playlist End Behavior",
							       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	obs_property_list_add_int(peb_property, "Stop", STOP);
	obs_property_list_add_int(peb_property, "Loop", LOOP);
	obs_property_list_add_int(peb_property, "Loop Specific Media", LOOP_SPECIFIC_MEDIA);
	obs_property_list_add_int(peb_property, "Loop Last Media", LOOP_LAST_MEDIA);

	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");

	obs_properties_add_bool(props, "debug", "Debug");
	return props;
}

void playlist_update(void *data, obs_data_t *settings)
{
	update_playlist_data(settings);
}

/**
 * @brief Updates the playlist data.
 * @param settings The settings of the playlist source.
 * @return void
 */
void update_playlist_data(obs_data_t *settings)
{
	// playlist_data.loop = obs_data_get_array(settings, "loop");
	// obs_log(LOG_INFO, playlist_data.loop ? "true" : "false");

	// playlist_data.playlist_start_behavior = obs_data_get_int(settings, "playlist_start_behavior");
	// obs_log(LOG_INFO, "Start Behavior: %zu", playlist_data.playlist_start_behavior);

	// playlist_data.playlist_end_behavior = obs_data_get_int(settings, "playlist_end_behavior");
	// obs_log(LOG_INFO, "end Behavior: %zu", playlist_data.playlist_end_behavior);

	// obs_data_array_t *obs_playlist = obs_data_get_array(settings, "playlist");

	// if (playlist_data.playlist.size > 0) {
	// 	free_string_array(&playlist_data.playlist);
	// }
	// size_t array_size = obs_data_array_count(obs_playlist);
	// if (array_size > 0) {
	// 	init_string_array(&playlist_data.playlist, 2); // Start with a small initial capacity
	// 	for (size_t i = 0; i < array_size; ++i) {
	// 		// Convert element to string (single character)
	// 		obs_data_t *data = obs_data_array_item(obs_playlist, i);
	// 		const char *element = obs_data_get_string(data, "value");
	// 		add_string(&playlist_data.playlist, element);

	// 		obs_data_release(data);
	// 		// obs_log(LOG_INFO, obs_array_to_string(playlist_data.playlist));
	// 		// obs_log(LOG_INFO, obs_array_to_string(playlist_data.playlist, 90));
	// 	}

	// 	// obs_log_string_array(LOG_INFO, &playlist_data.playlist, 90, "    ");
	// }
}

void playlist_tick(void *data, float seconds)
{
	struct PlaylistSource *playlist_data = data;
	// obs_log(LOG_INFO, playlist_data->debug ? "true" : "false");
	// obs_frontend_get_current_scene();
}
#pragma endregion