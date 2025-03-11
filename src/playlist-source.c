#include "../include/playlist-source.h"

const char *playlist_source_name(void *data)
{
	return "Playlist"; // This should match the filename (without extension) in data/
}

struct playlist_source *playlist = {0};
void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);
	obs_log(LOG_INFO, "we made it");

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
	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");
	obs_properties_add_bool(props, "loop", "Loop");
	return props;
}

void playlist_update(void *data, obs_data_t *settings) {}

void playlist_tick(void *data, float seconds) {}