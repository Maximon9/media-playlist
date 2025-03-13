#pragma region Main
#include "../include/sources/playlist-source.h"

const char *playlist_source_name(void *data)
{
	return "Playlist"; // This should match the filename (without extension) in data/
}

void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct PlaylistSource *playlist_data = bzalloc(sizeof(*playlist_data));

	playlist_data->source = source;
	playlist_data->all_media = NULL;
	playlist_data->current_media = NULL;
	playlist_data->current_media_index = 0;

	update_playlist_data(playlist_data, settings);

	return playlist_data;
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
	struct PlaylistSource *playlist_data = data;
	update_playlist_data(playlist_data, settings);
}

/**
 * @brief Updates the playlist data.
 * @param settings The settings of the playlist source.
 * @return void
 */
void update_playlist_data(struct PlaylistSource *playlist_data, obs_data_t *settings)
{
	playlist_data->debug = obs_data_get_bool(settings, "debug");
	if (playlist_data->debug) {
		obs_log(LOG_INFO, "Debug: %s", playlist_data->debug ? "true" : "false");
	}

	playlist_data->playlist_start_behavior = obs_data_get_int(settings, "playlist_start_behavior");
	if (playlist_data->debug) {
		obs_log(LOG_INFO, "Start Behavior: %s", StartBehaviorName[playlist_data->playlist_start_behavior]);
	}

	playlist_data->playlist_end_behavior = obs_data_get_int(settings, "playlist_end_behavior");
	if (playlist_data->debug) {
		obs_log(LOG_INFO, "end Behavior: %s", EndBehaviorName[playlist_data->playlist_end_behavior]);
	}

	if (playlist_data->all_media != NULL && playlist_data->all_media->size > 0) {
		free_media_array(playlist_data->all_media);
	}

	playlist_data->all_media = create_meda_file_data_array_from_obs_array(obs_data_get_array(settings, "playlist"));
	if (playlist_data->all_media != NULL && playlist_data->debug) {
		obs_log_media_array(LOG_INFO, playlist_data->all_media, 90, "    ");
	}
}

void playlist_tick(void *data, float seconds)
{
	struct PlaylistSource *playlist_data = data;
	// if (playlist_data->debug) {
	// 	obs_log_media_array(LOG_INFO, playlist_data->all_media, 90, "    ");
	// }
	// obs_frontend_get_current_scene();
}
#pragma endregion