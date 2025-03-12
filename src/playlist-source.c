#include "../include/playlist-source.h"
#include "../include/utils.h"

const char *playlist_source_name(void *data)
{
	return "Playlist"; // This should match the filename (without extension) in data/
}

struct playlist_source playlist_data = {.playlist = NULL, .loop = false};
void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);
	obs_log(LOG_INFO, "we made it");
	struct playlist_source *m_playlist = bzalloc(sizeof(playlist_data));
	return m_playlist;
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
	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");
	obs_properties_add_bool(props, "loop", "Loop");
	return props;
}

void playlist_update(void *data, obs_data_t *settings)
{
	playlist_data.loop = obs_data_get_bool(settings, "loop");
	if (playlist_data.playlist != NULL) {
		obs_data_array_release(playlist_data.playlist);
	}
	playlist_data.playlist = obs_data_get_array(settings, "playlist");
	obs_log(LOG_INFO, playlist_data.loop ? "true" : "false");
	// obs_log(LOG_INFO, array_to_string(playlist_data.playlist));
	obs_data_t *test_data = obs_data_array_item(playlist_data.playlist, 0);
	obs_log(LOG_INFO, obs_data_get_string(test_data, "value"));
}

void playlist_tick(void *data, float seconds) {}