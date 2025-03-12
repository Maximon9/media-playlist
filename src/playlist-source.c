#include "../include/playlist-source.h"

const char *playlist_source_name(void *data)
{
	return "Playlist"; // This should match the filename (without extension) in data/
}

struct playlist_source playlist_data = {.playlist = NULL, .loop = false};
void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);
	obs_log(LOG_INFO, "we made it");
	// struct playlist_source *m_playlist = bzalloc(sizeof(playlist_data));
	update_playlist_data(settings);
	return &playlist_data;
}

void playlist_source_destroy(void *data)
{
	if (playlist_data.playlist.size > 0) {
		free_string_array(&playlist_data.playlist);
	}
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
	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");
	obs_properties_add_bool(props, "loop", "Loop");
	return props;
}

void playlist_update(void *data, obs_data_t *settings)
{
	update_playlist_data(settings);
}

void update_playlist_data(obs_data_t *settings)
{
	playlist_data.loop = obs_data_get_bool(settings, "loop");
	obs_log(LOG_INFO, playlist_data.loop ? "true" : "false");

	obs_data_array_t *obs_playlist = obs_data_get_array(settings, "playlist");

	if (playlist_data.playlist.size > 0) {
		free_string_array(&playlist_data.playlist);
	}
	size_t array_size = obs_data_array_count(obs_playlist);
	if (array_size > 0) {
		init_string_array(&playlist_data.playlist, 2); // Start with a small initial capacity
		for (size_t i = 0; i < array_size; ++i) {
			// Convert element to string (single character)
			obs_data_t *data = obs_data_array_item(obs_playlist, i);
			const char *element = obs_data_get_string(data, "value");
			add_string(&playlist_data.playlist, element);

			obs_data_release(data);
			// obs_log(LOG_INFO, obs_array_to_string(playlist_data.playlist));
			// obs_log(LOG_INFO, obs_array_to_string(playlist_data.playlist, 90));
		}

		obs_log(LOG_INFO, "The size is: %zu", playlist_data.playlist.size);
		char *result = stringify_string_array(&playlist_data.playlist, 90, "    ");
		obs_log(LOG_INFO, result);
		free(result);
	}
}
void playlist_activate(void *data)
{
	obs_log(LOG_INFO, "activated");
}

void playlist_deactivate(void *data)
{
	obs_log(LOG_INFO, "deactivated");
}

void playlist_tick(void *data, float seconds) {}