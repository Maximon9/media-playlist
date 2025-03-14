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
	obs_data_set_default_int(settings, "start_index", 0);
	obs_data_set_default_int(settings, "end_index", 0);
	obs_data_set_default_int(settings, "playlist_start_behavior", 0);
	obs_data_set_default_int(settings, "playlist_end_behavior", 0);
	obs_data_set_default_bool(settings, "debug", false);
}

obs_properties_t *make_playlist_properties(struct PlaylistSource *playlist_data)
{
	obs_properties_t *props = obs_properties_create();

	obs_properties_add_int_slider(props, "start_index", "Start Index", 0, playlist_data->end_index, 1);

	obs_properties_add_int_slider(props, "end_index", "End Index", playlist_data->start_index,
				      (int)(playlist_data->all_media->size - 1), 1);

	obs_property_t *psb_property = obs_properties_add_list(
		props, "playlist_start_behavior", "Playlist Start Behavior", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	long long i = 0;
	const char *name = StartBehavior[i];
	while (name != "") {
		obs_property_list_add_int(psb_property, name, i);
		i++;
		name = StartBehavior[i];
	}

	obs_property_t *peb_property = obs_properties_add_list(props, "playlist_end_behavior", "Playlist End Behavior",
							       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	i = 0;
	name = EndBehavior[i];
	while (name != "") {
		obs_property_list_add_int(peb_property, name, i);
		i++;
		name = EndBehavior[i];
	}

	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");

	obs_properties_add_bool(props, "debug", "Debug");
	return props;
}

obs_properties_t *playlist_get_properties(void *data)
{
	obs_log(LOG_INFO, "Getting Properties");
	return make_playlist_properties(data);
}

void playlist_update(void *data, obs_data_t *settings)
{
	struct PlaylistSource *playlist_data = data;
	update_playlist_data(playlist_data, settings);

	// Ensure properties are refreshed or updated if necessary
	// obs_properties_t *props = make_playlist_properties(playlist_data);
	// obs_source_update(playlist_data->source, props);
}

/**
 * @brief Updates the playlist data.
 * @param settings The settings of the playlist source.
 * @return void
 */
void update_playlist_data(struct PlaylistSource *playlist_data, obs_data_t *settings)
{
	playlist_data->start_index = (int)obs_data_get_int(settings, "start_index");
	playlist_data->end_index = (int)obs_data_get_int(settings, "end_index");

	playlist_data->debug = obs_data_get_bool(settings, "debug");
	if (playlist_data->debug) {
		obs_log(LOG_INFO, "Debug: %s", playlist_data->debug ? "true" : "false");
	}

	playlist_data->playlist_start_behavior = obs_data_get_int(settings, "playlist_start_behavior");
	if (playlist_data->debug) {
		obs_log(LOG_INFO, "Start Behavior: %s", StartBehavior[playlist_data->playlist_start_behavior]);
	}

	playlist_data->playlist_end_behavior = obs_data_get_int(settings, "playlist_end_behavior");
	if (playlist_data->debug) {
		obs_log(LOG_INFO, "end Behavior: %s", EndBehavior[playlist_data->playlist_end_behavior]);
	}

	size_t previous_size = 0;
	// obs_log(LOG_INFO, "Sizes: %d", previous_size);
	if (playlist_data->all_media != NULL && playlist_data->all_media->size > 0) {
		previous_size = playlist_data->all_media->size;
		free_media_array(playlist_data->all_media);
	}

	playlist_data->all_media = create_meda_file_data_array_from_obs_array(obs_data_get_array(settings, "playlist"));
	// obs_log(LOG_INFO, "Sizes: %d, %s", previous_size, playlist_data->all_media->size);
	if (playlist_data->end_index == previous_size - 1) {
		playlist_data->end_index = (int)(playlist_data->all_media->size - 1);
		obs_data_set_int(settings, "end_index", playlist_data->end_index);
	}

	if (playlist_data->all_media != NULL && playlist_data->debug) {
		obs_log_media_array(LOG_INFO, playlist_data->all_media, 90, "    ");
	}

	if (playlist_data->start_index < 0) {
		playlist_data->start_index = 0;
	} else if (playlist_data->start_index >= playlist_data->end_index) {
		playlist_data->start_index = playlist_data->end_index;
	}

	if (playlist_data->end_index < playlist_data->start_index) {
		playlist_data->end_index = playlist_data->start_index;
	} else if (playlist_data->end_index >= playlist_data->all_media->size) {
		playlist_data->end_index = (int)(playlist_data->all_media->size - 1);
	}
	obs_data_set_int(settings, "start_index", playlist_data->start_index);
	obs_data_set_int(settings, "end_index", playlist_data->end_index);

	// obs_properties_t *props = make_playlist_properties();
	obs_source_update_properties(playlist_data->source);
	// obs_source_update(playlist_data->source, settings);
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