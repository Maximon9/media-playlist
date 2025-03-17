#pragma region Main
#include "../include/sources/playlist-source.h"

const char *playlist_source_name(void *data)
{
	return "Playlist"; // This should match the filename (without extension) in data/
}

void playlist_on_scene_switch(enum obs_frontend_event event, void *private_data)
{
	if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED) {
		// Example: Get the current scene and log its name
		obs_source_t *scene_source = obs_frontend_get_current_scene();
		struct PlaylistSource *playlist_data = private_data;

		const char *source_name = obs_source_get_name(playlist_data->source);

		obs_scene_t *scene = obs_scene_from_source(scene_source);

		// Check if a source with the name "my_source" exists in the scene
		obs_sceneitem_t *source = obs_scene_find_source_recursive(scene, source_name);

		if (source) {
			playlist_data->run = true;
			obs_log(LOG_INFO, "We did it cool");
		} else {
			playlist_data->run = false;
		}
	}
}

void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct PlaylistSource *playlist_data = bzalloc(sizeof(*playlist_data));

	playlist_data->source = source;

	playlist_data->all_media = NULL;

	playlist_data->start_index = 0;
	playlist_data->end_index = 0;

	playlist_data->current_media = NULL;
	playlist_data->current_media_index = 0;
	playlist_data->loop_index = 0;
	playlist_data->infinite = true;
	playlist_data->loop_count = 0;

	playlist_data->run = false;

	playlist_data->current_media_source =
		obs_source_create_private("ffmpeg_source", "current_media_source", settings);

	if (playlist_data->current_media_source) {
		obs_source_add_active_child(source, playlist_data->current_media_source);
	}

	update_playlist_data(playlist_data, settings);

	return playlist_data;
}

void playlist_source_destroy(void *data)
{
	obs_log(LOG_INFO, "It got deaded");

	struct PlaylistSource *playlist_data = data;

	if (playlist_data->current_media_source) {
		obs_source_release(playlist_data->current_media_source);
	}

	free_media_array(playlist_data->all_media);

	if (playlist_data->current_media != NULL) {
		free(playlist_data->current_media);
	}

	bfree(playlist_data);
	obs_source_release(playlist_data->source);
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
	// obs_data_set_default_int(settings, "start_index", 0);
	// obs_data_set_default_int(settings, "end_index", 0);
	// obs_data_set_default_int(settings, "playlist_start_behavior", 0);
	// obs_data_set_default_int(settings, "playlist_end_behavior", 0);
	// obs_data_set_default_int(settings, "loop_index", 0);
	obs_data_set_default_bool(settings, "infinite", true);
	// obs_data_set_default_int(settings, "loop_count", 0);
	obs_data_set_default_bool(settings, "debug", false);
}

obs_properties_t *make_playlist_properties(struct PlaylistSource *playlist_data)
{
	obs_properties_t *props = obs_properties_create();

	int all_media_size = 0;

	if (playlist_data->all_media != NULL) {
		all_media_size = (int)playlist_data->all_media->size;
	}

	int last_index = all_media_size - 1;

	if (last_index < 0) {
		last_index += 1;
	}

	obs_properties_add_int_slider(props, "start_index", "Start Index", 0, last_index, 1);

	obs_properties_add_int_slider(props, "end_index", "End Index", 0, last_index, 1);

	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");

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

	if (playlist_data->playlist_end_behavior == LOOP_AT_INDEX) {
		obs_properties_add_int_slider(props, "loop_index", "Loop Index", 0, last_index, 1);
	}
	if (playlist_data->playlist_end_behavior == LOOP_AT_INDEX ||
	    playlist_data->playlist_end_behavior == LOOP_AT_END) {
		obs_properties_add_bool(props, "infinite", "infinite");
		if (playlist_data->infinite == false) {
			obs_properties_add_int(props, "loop_count", "Loop Count", 0, INT_MAX, 1);
		}
	}

	obs_properties_add_bool(props, "debug", "Debug");
	return props;
}

obs_properties_t *playlist_get_properties(void *data)
{
	return make_playlist_properties(data);
}

/**
 * @brief Updates the playlist data.
 * @param settings The settings of the playlist source.
 * @return void
 */
void update_playlist_data(struct PlaylistSource *playlist_data, obs_data_t *settings)
{
	bool update_properties = false;
	// int previous_start_index = playlist_data->start_index;
	int previous_end_index = playlist_data->end_index;

	playlist_data->start_index = (int)obs_data_get_int(settings, "start_index");
	playlist_data->end_index = (int)obs_data_get_int(settings, "end_index");

	playlist_data->debug = obs_data_get_bool(settings, "debug");
	if (playlist_data->debug == true) {
		obs_log(LOG_INFO, "Debug: %s", playlist_data->debug ? "true" : "false");
	}

	playlist_data->playlist_start_behavior = obs_data_get_int(settings, "playlist_start_behavior");
	if (playlist_data->debug == true) {
		obs_log(LOG_INFO, "Start Behavior: %s", StartBehavior[playlist_data->playlist_start_behavior]);
	}

	enum EndBehavior playlist_end_behavior = obs_data_get_int(settings, "playlist_end_behavior");
	if (playlist_data->playlist_end_behavior != playlist_end_behavior) {
		update_properties = true;
	}

	playlist_data->playlist_end_behavior = playlist_end_behavior;
	if (playlist_data->debug == true) {
		obs_log(LOG_INFO, "end Behavior: %s", EndBehavior[playlist_data->playlist_end_behavior]);
	}

	bool previous_size_initialized = false;
	size_t previous_size = 0;
	// obs_log(LOG_INFO, "Sizes: %d", previous_size);
	if (playlist_data->all_media != NULL && playlist_data->all_media->size > 0) {
		previous_size = playlist_data->all_media->size;
		previous_size_initialized = true;
		free_media_array(playlist_data->all_media);
	}

	playlist_data->all_media = obs_data_array_retain(obs_data_get_array(settings, "playlist"));
	int all_media_size = 0;

	if (playlist_data->all_media != NULL) {
		all_media_size = (int)playlist_data->all_media->size;
	}

	// obs_log(LOG_INFO, "Sizes: %d, %s", previous_size, playlist_data->all_media->size);

	bool update_start_index_ui = false;
	bool update_end_index_ui = false;

	int last_index = all_media_size - 1;

	if (last_index < 0) {
		last_index += 1;
	}

	if (playlist_data->playlist_end_behavior == LOOP_AT_INDEX) {
		bool update_loop_index_ui = false;

		int loop_index = (int)obs_data_get_int(settings, "loop_index");
		bool infinite = obs_data_get_bool(settings, "infinite");
		int loop_count = (int)obs_data_get_int(settings, "loop_count");

		if (playlist_data->debug == true) {
			obs_log(LOG_INFO, "Infinite New Value: %s", loop_index);
			obs_log(LOG_INFO, "Infinite New Value: %s", infinite == true ? "true" : "false");
			obs_log(LOG_INFO, "Infinite New Value: %s", loop_count);
		}
		if (infinite != playlist_data->infinite) {
			update_properties = true;
		}

		playlist_data->loop_index = loop_index;
		playlist_data->infinite = infinite;
		playlist_data->loop_count = loop_count;

		if (playlist_data->loop_index < 0) {
			playlist_data->loop_index = playlist_data->start_index;
			update_loop_index_ui = true;
			update_properties = true;
		} else if (playlist_data->end_index >= all_media_size) {
			playlist_data->loop_index = last_index;
			update_loop_index_ui = true;
			update_properties = true;
		}

		if (update_loop_index_ui) {
			obs_data_set_int(settings, "loop_index", playlist_data->loop_index);
		}
	}

	if (previous_size_initialized == true) {
		if (playlist_data->all_media->size != 0 && previous_size != playlist_data->all_media->size) {
			update_properties = true;
			if (playlist_data->end_index == previous_size - 1) {
				playlist_data->end_index = last_index;
				update_end_index_ui = true;
			}
		}
	}

	if (playlist_data->all_media != NULL && playlist_data->debug) {
		obs_log_media_array(LOG_INFO, playlist_data->all_media, 90, "    ");
	}

	if (playlist_data->start_index > previous_end_index) {
		playlist_data->start_index = previous_end_index;
		update_start_index_ui = true;
		update_properties = true;
	}

	if (playlist_data->end_index < playlist_data->start_index) {
		playlist_data->end_index = playlist_data->start_index;
		update_end_index_ui = true;
		update_properties = true;
	} else if (playlist_data->end_index >= all_media_size) {
		playlist_data->end_index = last_index;
		update_end_index_ui = true;
		update_properties = true;
	}

	if (update_start_index_ui) {
		obs_data_set_int(settings, "start_index", playlist_data->start_index);
	}

	if (update_end_index_ui) {
		obs_data_set_int(settings, "end_index", playlist_data->end_index);
	}

	if (update_properties == true) {
		obs_source_update_properties(playlist_data->source);
	}
}

void playlist_update(void *data, obs_data_t *settings)
{
	struct PlaylistSource *playlist_data = data;
	update_playlist_data(playlist_data, settings);

	// Ensure properties are refreshed or updated if necessary
	// obs_properties_t *props = make_playlist_properties(playlist_data);
	// obs_source_update(playlist_data->source, props);
}

void playlist_activate(void *data)
{
	obs_log(LOG_INFO, "playlist_activate");
	struct PlaylistSource *playlist_data = data;
	obs_frontend_add_event_callback(playlist_on_scene_switch, playlist_data);
}

void playlist_deactivate(void *data)
{
	obs_log(LOG_INFO, "playlist_deactivate");
	struct PlaylistSource *playlist_data = data;
	obs_frontend_remove_event_callback(playlist_on_scene_switch, playlist_data);
}

void playlist_tick(void *data, float seconds)
{
	struct PlaylistSource *playlist_data = data;
	// if (playlist_data->debug) {
	// 	obs_log_media_array(LOG_INFO, playlist_data->all_media, 90, "    ");
	// }
	// obs_frontend_get_current_scene();
}

void playlist_video_render(void *data, gs_effect_t *effect)
{
	// obs_log(LOG_INFO, "video_render");
}

void playlist_save(void *data, obs_data_t *settings)
{
	obs_log(LOG_INFO, "playlist_save");
}

void playlist_load(void *data, obs_data_t *settings)
{
	obs_log(LOG_INFO, "playlist_load");
}

void media_play_pause(void *data, bool pause)
{
	obs_log(LOG_INFO, "media_play_pause");
}

void media_restart(void *data)
{
	obs_log(LOG_INFO, "media_restart");
}

void media_stop(void *data)
{
	obs_log(LOG_INFO, "media_stop");
}

void media_next(void *data)
{
	obs_log(LOG_INFO, "media_next");
}

void media_previous(void *data)
{
	obs_log(LOG_INFO, "media_previous");
}

int64_t media_get_duration(void *data)
{
	obs_log(LOG_INFO, "media_get_duration");
	return 0;
}

int64_t media_get_time(void *data)
{
	obs_log(LOG_INFO, "media_get_time");
	return 0;
}

void media_set_time(void *data, int64_t miliseconds)
{
	obs_log(LOG_INFO, "media_set_time");
}

enum obs_media_state media_get_state(void *data)
{
	obs_log(LOG_INFO, "media_get_state");
	return OBS_MEDIA_STATE_NONE;
}

#pragma endregion
