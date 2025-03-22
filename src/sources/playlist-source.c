#pragma region Main
#include "../include/sources/playlist-source.h"

#define S_FFMPEG_LOG_CHANGES "log_changes"
#define S_FFMPEG_RESTART_PLAYBACK_ON_ACTIVE "restart_on_activate"
#define S_FFMPEG_LOCAL_FILE "local_file"
#define S_FFMPEG_IS_LOCAL_FILE "is_local_file"
#define S_CURRENT_MEDIA_INDEX "current_media_index"

#pragma region Media Functions
const char *get_current_media_input(obs_data_t *settings)
{
	const char *path = obs_data_get_string(settings, S_FFMPEG_LOCAL_FILE);
	return path;
}

void refresh_queue_list(struct PlaylistSource *playlist_data)
{
	clear_media_array(&playlist_data->queue);

	for (size_t i = 0; i < playlist_data->all_media.size; i++) {
		const MediaFileData *media_file_data = get_media(&playlist_data->all_media, i);

		const MediaFileData new_entry = create_media_file_data_with_all_info(
			media_file_data->path, media_file_data->filename, media_file_data->name, media_file_data->ext,
			media_file_data->index);

		push_media_file_data_back(&playlist_data->queue, new_entry);
	}
}

void playlist_media_source_ended(void *data, calldata_t *callback)
{
	UNUSED_PARAMETER(callback);
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->queue.size > 0) {

		// switch (playlist_data->end_index)
		// {
		// case END_BEHAVIOR_STOP:
		// 	obs_source_media_stop(playlist_data->source);
		// 	break;
		// case END_BEHAVIOR_LOOP:
		// 	// obs_source_media_stop(playlist_data->source);
		// 	break;
		// case END_BEHAVIOR_LOOP_AT_INDEX:
		// 	// obs_source_media_stop(playlist_data->source);
		// 	break;
		// case END_BEHAVIOR_LOOP_AT_END:
		// 	// obs_source_media_stop(playlist_data->source);
		// 	break;
		// default:
		// 	break;
		// }
		obs_source_media_next(playlist_data->source);
	} else {
		switch (playlist_data->end_index) {
		case END_BEHAVIOR_STOP:
			obs_source_media_stop(playlist_data->source);
			break;
		case END_BEHAVIOR_LOOP:
			// obs_source_media_stop(playlist_data->source);
			break;
		case END_BEHAVIOR_LOOP_AT_INDEX:
			// obs_source_media_stop(playlist_data->source);
			break;
		case END_BEHAVIOR_LOOP_AT_END:
			// obs_source_media_stop(playlist_data->source);
			break;
		default:
			break;
		}
	}

	obs_log(LOG_INFO, "We ended the media");
}

void playlist_queue(struct PlaylistSource *playlist_data)
{
	if (!playlist_data || playlist_data->all_media.size <= 0 || !playlist_data->media_source ||
	    !playlist_data->media_source_settings)
		return;

	if (playlist_data->queue.size <= 0)
		return;

	// Get video file path from the array
	const MediaFileData *media_data = get_media(&playlist_data->queue, 0);

	if (!media_data)
		return;

	// Set file path in ffmpeg_source
	obs_data_set_string(playlist_data->media_source_settings, S_FFMPEG_LOCAL_FILE, media_data->path);

	// obs_log(LOG_INFO, " PRINTING: \n%s", obs_data_get_json_pretty(playlist_data->media_source_settings));

	obs_source_update(playlist_data->media_source, playlist_data->media_source_settings);
	// obs_source_media_restart(playlist_data->source);

	// enum obs_media_state state = obs_source_media_get_state(playlist_data->media_source);
	// if (state != OBS_MEDIA_STATE_STOPPED) {
	// 	obs_log(LOG_INFO, "ok we stopped %d, %s", state, state == OBS_MEDIA_STATE_PLAYING ? "true" : "false");
	// 	obs_source_media_stop(playlist_data->source);
	// }
	// Set it as active
	// obs_source_media_play_pause(playlist_data->source, false);
}

void playlist_queue_restart(struct PlaylistSource *playlist_data)
{
	if (!playlist_data || playlist_data->all_media.size <= 0 || !playlist_data->media_source ||
	    !playlist_data->media_source_settings)
		return;

	if (playlist_data->queue.size <= 0)
		return;

	// Get video file path from the array
	const MediaFileData *media_data = get_media(&playlist_data->queue, 0);

	if (!media_data)
		return;

	// Set file path in ffmpeg_source
	obs_data_set_string(playlist_data->media_source_settings, S_FFMPEG_LOCAL_FILE, media_data->path);

	// obs_log(LOG_INFO, " PRINTING: \n%s", obs_data_get_json_pretty(playlist_data->media_source_settings));

	obs_source_update(playlist_data->media_source, playlist_data->media_source_settings);
	obs_source_media_restart(playlist_data->source);
}

void clear_any_media_playing(struct PlaylistSource *playlist_data)
{
	if (!playlist_data->media_source || !playlist_data->media_source_settings)
		return;

	// Set file path in ffmpeg_source
	obs_data_set_string(playlist_data->media_source_settings, S_FFMPEG_LOCAL_FILE, "");

	// obs_log(LOG_INFO, " PRINTING: \n%s", obs_data_get_json_pretty(playlist_data->media_source_settings));

	obs_source_update(playlist_data->media_source, playlist_data->media_source_settings);
	obs_source_media_stop(playlist_data->source);
}

void playlist_audio_callback(void *data, obs_source_t *source, const struct audio_data *audio_data, bool muted)
{
	UNUSED_PARAMETER(muted);
	UNUSED_PARAMETER(source);

	struct PlaylistSource *playlist_data = data;

	pthread_mutex_lock(&playlist_data->audio_mutex);

	size_t size = audio_data->frames * sizeof(float);
	for (size_t i = 0; i < playlist_data->num_channels; i++) {
		deque_push_back(&playlist_data->audio_data[i], audio_data->data[i], size);
	}
	deque_push_back(&playlist_data->audio_frames, &audio_data->frames, sizeof(audio_data->frames));
	deque_push_back(&playlist_data->audio_timestamps, &audio_data->timestamp, sizeof(audio_data->timestamp));

	pthread_mutex_unlock(&playlist_data->audio_mutex);
}

bool uses_song_history_limit(struct PlaylistSource *playlist_data)

{
	return (playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_INDEX ||
		playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_END ||
		(playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP && playlist_data->shuffle_queue == true));
}
#pragma endregion

#pragma region Property Managment

obs_properties_t *make_playlist_properties(struct PlaylistSource *playlist_data)
{
	obs_properties_t *props = obs_properties_create();

	int all_media_size = (int)playlist_data->all_media.size;

	int last_index = all_media_size - 1;

	if (last_index < 0) {
		last_index += 1;
	}

	obs_properties_add_int(props, "queue_list_size", "Queue List Size", 5, 20, 1);

	pthread_mutex_lock(&playlist_data->mutex);

	// obs_log_media_array(LOG_INFO, "Queue Array:\n", &playlist_data->queue, 0, "    ", MEDIA_STRINGIFY_TYPE_NAME);

	char *result = stringify_media_queue_array(&playlist_data->queue, playlist_data->queue_list_size, "    ",
						   MEDIA_STRINGIFY_TYPE_NAME);

	char *concat_result = concat_mem_string("Queue: ", result);

	obs_properties_add_text(props, "queue", concat_result, OBS_TEXT_INFO);

	free(result);
	free(concat_result);

	pthread_mutex_unlock(&playlist_data->mutex);

	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");

	obs_properties_add_bool(props, "shuffle_queue", "Shuffle Queue");

	obs_property_t *psb_property = obs_properties_add_list(
		props, "playlist_start_behavior", "Playlist Start Behavior", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	add_enums_to_property_list(psb_property, StartBehavior, 2);

	obs_property_t *peb_property = obs_properties_add_list(props, "playlist_end_behavior", "Playlist End Behavior",
							       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	add_enums_to_property_list(peb_property, EndBehavior, 2);

	if (playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_INDEX) {
		obs_properties_add_int_slider(props, "loop_index", "Loop Index", 0, last_index, 1);
	}
	if (playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_INDEX ||
	    playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_END) {
		obs_properties_add_bool(props, "infinite", "Infinite");
		if (playlist_data->infinite == false) {
			obs_properties_add_int(props, "loop_count", "Loop Count", 0, INT_MAX, 1);

			obs_property_t *leb_property = obs_properties_add_list(props, "loop_end_behavior",
									       "Loop End Behavior", OBS_COMBO_TYPE_LIST,
									       OBS_COMBO_FORMAT_INT);
			add_enums_to_property_list(leb_property, LoopEndBehavior, 3);
		}
	}

	obs_properties_add_int(props, "song_history_limit", "Song History Limit", 0, INT_MAX, 1);

	obs_properties_add_bool(props, "debug", "Debug");
	return props;
}

/**
 * @brief Updates the playlist data.
 * @param settings The settings of the playlist source.
 * @return void
 */
void update_playlist_data(struct PlaylistSource *playlist_data, obs_data_t *settings)
{
	bool update_properties = false;

	int queue_list_size = (int)obs_data_get_int(settings, "queue_list_size");
	if (playlist_data->queue_list_size != queue_list_size) {
		update_properties = true;
	}

	playlist_data->queue_list_size = queue_list_size;

	if (playlist_data->debug == true) {
		obs_log(LOG_INFO, "Queue List Size: %d", playlist_data->queue_list_size);
	}

	pthread_mutex_lock(&playlist_data->mutex);

	MediaFileDataArray new_media_array;

	init_media_array(&new_media_array, 2);

	obs_data_media_array_retain(&new_media_array, obs_data_get_array(settings, "playlist"));

	size_t old_media_size = playlist_data->all_media.size;

	size_t new_media_size = new_media_array.size;

	bool media_arrays_are_equal = compare_media_file_data_arrays(&new_media_array, &playlist_data->all_media);

	clear_media_array(&playlist_data->all_media);
	move_media_array(&playlist_data->all_media, &new_media_array);

	// obs_log(LOG_INFO, "Media Array Changed: %s Initialized Media: %s",
	// 	media_arrays_are_equal == false ? "true" : "false",
	// 	playlist_data->all_media_initialized == true ? "true" : "false");

	if (media_arrays_are_equal == false && playlist_data->all_media_initialized == true) {
		update_properties = true;

		if (old_media_size == 0 && new_media_size != 0) {
			refresh_queue_list(playlist_data);
			playlist_queue_restart(playlist_data);
		} else {
			SizeTArray existing_indices;

			da_init(existing_indices);

			const size_t *queue_size = &playlist_data->queue.size;

			if (queue_size != NULL) {
				for (size_t i = 0; i < *queue_size; i++) {
					size_t queue_index = get_media(&playlist_data->queue, i)->index;
					const MediaFileData *media_file_data =
						get_media(&playlist_data->all_media, queue_index);
					if (media_file_data == NULL) {
						da_push_back(existing_indices, &queue_index);
					}
				}
			}

			if (existing_indices.num > 0) {
				MediaFileDataArray new_queue;
				init_media_array(&new_queue, 2);

				// for (size_t i = 0; i < existing_indices.num; i++) {
				// 	const size_t queue_index = existing_indices.array[i];
				// 	obs_log(LOG_INFO, "Existing Index: %d", queue_index);
				// const MediaFileData *media_file_data = get_media(&playlist_data->all_media, i);

				// const MediaFileData new_entry = create_media_file_data_with_all_info(
				// 	media_file_data->path, media_file_data->filename, media_file_data->name,
				// 	media_file_data->ext, media_file_data->index);

				// push_media_file_data_back(&new_queue, new_entry);
				// }

				move_media_array(&playlist_data->queue, &new_queue);
			}

			da_free(existing_indices);

			const char *media_input = get_current_media_input(playlist_data->media_source_settings);
			const char *current_queue_path = get_media(&playlist_data->queue, 0)->path;

			obs_log(LOG_INFO, "Comparing Strings: %s",
				strcmp(media_input, current_queue_path) == 0 ? "true" : "false");

			if (strcmp(media_input, current_queue_path) == 0) {
				playlist_queue(playlist_data);
			} else {
				playlist_queue_restart(playlist_data);
			}
		}
	}
	playlist_data->all_media_initialized = true;

	pthread_mutex_unlock(&playlist_data->mutex);

	if (playlist_data->debug == true) {
		obs_log_media_array(LOG_INFO, "All Media:\n", &playlist_data->all_media, 0, "    ",
				    MEDIA_STRINGIFY_TYPE_FILENAME);
	}

	bool shuffle_queue = obs_data_get_bool(settings, "shuffle_queue");

	if (shuffle_queue != playlist_data->shuffle_queue) {
		update_properties = true;
	}

	playlist_data->shuffle_queue = shuffle_queue;

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
		obs_log(LOG_INFO, "End Behavior: %s", EndBehavior[playlist_data->playlist_end_behavior]);
	}

	int all_media_size = (int)playlist_data->all_media.size;

	int last_index = all_media_size - 1;

	if (last_index < 0) {
		last_index += 1;
	}

	if (playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_INDEX ||
	    playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_END) {
		bool infinite = obs_data_get_bool(settings, "infinite");
		if (infinite != playlist_data->infinite) {
			update_properties = true;
		}
		playlist_data->infinite = infinite;
		if (playlist_data->infinite) {
			playlist_data->loop_count = (int)obs_data_get_int(settings, "loop_count");
			playlist_data->loop_end_behavior = obs_data_get_int(settings, "loop_end_behavior");
		}
	}
	if (playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_INDEX) {
		playlist_data->loop_index = (int)obs_data_get_int(settings, "loop_index");
		if (playlist_data->debug == true) {
			obs_log(LOG_INFO, "Loop Index: %d", playlist_data->loop_index);
		}

		bool update_loop_index_ui = false;

		if (playlist_data->loop_index < 0) {
			playlist_data->loop_index = 0;
			update_loop_index_ui = true;
		} else if (playlist_data->loop_index >= all_media_size) {
			playlist_data->loop_index = last_index;
			update_loop_index_ui = true;
		}

		if (update_loop_index_ui) {
			update_properties = true;
			obs_data_set_int(settings, "loop_index", playlist_data->loop_index);
		}
	}

	if (playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_INDEX ||
	    playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP_AT_END) {
		obs_log(LOG_INFO, "Infinite: %s", playlist_data->infinite == true ? "true" : "false");
		if (playlist_data->infinite == false && playlist_data->debug == true) {
			obs_log(LOG_INFO, "Loop Count: %d", playlist_data->loop_count);
		}
	}

	if (playlist_data->all_media.size != 0 && media_arrays_are_equal == false) {
		update_properties = true;
	}

	playlist_data->song_history_limit = (int)obs_data_get_int(settings, "song_history_limit");

	playlist_data->debug = obs_data_get_bool(settings, "debug");
	if (playlist_data->debug == true) {
		obs_log(LOG_INFO, "Debug: %s", playlist_data->debug ? "true" : "false");
	}

	if (update_properties == true) {
		obs_source_update_properties(playlist_data->source);
	}
}

#pragma endregion

#pragma region Playlist Main Functions

const char *playlist_source_name(void *data)
{
	return "Playlist"; // This should match the filename (without extension) in data/
}

void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct PlaylistSource *playlist_data = bzalloc(sizeof(*playlist_data));

	playlist_data->source = source;
	// playlist_data->media_source = NULL;
	// playlist_data->media_source_settings = NULL;

	playlist_data->media_source_settings = obs_data_create();

	obs_data_set_bool(playlist_data->media_source_settings, S_FFMPEG_LOG_CHANGES, false);
	obs_data_set_bool(playlist_data->media_source_settings, S_FFMPEG_RESTART_PLAYBACK_ON_ACTIVE, false);
	playlist_data->media_source =
		obs_source_create_private("ffmpeg_source", "Video Source", playlist_data->media_source_settings);

	obs_source_add_active_child(playlist_data->source, playlist_data->media_source);
	obs_source_add_audio_capture_callback(playlist_data->media_source, playlist_audio_callback, playlist_data);

	signal_handler_t *sh_media_source = obs_source_get_signal_handler(playlist_data->media_source);
	signal_handler_connect(sh_media_source, "media_ended", playlist_media_source_ended, playlist_data);

	// MediaFileDataArray all_media;

	// da_init(all_media);

	// playlist_data->all_media = &all_media;
	playlist_data->all_media_initialized = false;

	init_media_array(&playlist_data->all_media, 2);

	init_media_array(&playlist_data->queue, 2);

	init_media_array(&playlist_data->previous_queue, 2);

	// playlist_data->current_media = NULL;
	// playlist_data->current_media_index = 0;
	playlist_data->loop_index = 0;
	playlist_data->infinite = true;
	playlist_data->loop_count = 0;
	playlist_data->song_history_limit = 100;
	playlist_data->queue_list_size = 5;

	// pthread_mutex_init_value(&playlist_data->mutex);
	if (pthread_mutex_init(&playlist_data->mutex, NULL) != 0)
		goto error;

	// pthread_mutex_init_value(&playlist_data->audio_mutex);
	if (pthread_mutex_init(&playlist_data->audio_mutex, NULL) != 0)
		goto error;

	update_playlist_data(playlist_data, settings);

	return playlist_data;
error:
	playlist_source_destroy(playlist_data);
	return NULL;
}

void playlist_source_destroy(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_release(playlist_data->media_source);
	}
	if (playlist_data->media_source_settings != NULL) {
		obs_data_release(playlist_data->media_source_settings);
	}

	for (size_t i = 0; i < MAX_AUDIO_CHANNELS; i++) {
		deque_free(&playlist_data->audio_data[i]);
	}
	deque_free(&playlist_data->audio_frames);
	deque_free(&playlist_data->audio_timestamps);
	pthread_mutex_destroy(&playlist_data->mutex);
	pthread_mutex_destroy(&playlist_data->audio_mutex);

	free_media_array(&playlist_data->all_media);
	free_media_array(&playlist_data->queue);
	free_media_array(&playlist_data->previous_queue);
	// free_media_array(&playlist_data->queue);
	// free_media_array(&playlist_data->previous_queue);

	// if (playlist_data->current_media != NULL) {
	// 	free(playlist_data->current_media);
	// }

	bfree(playlist_data);
	obs_source_release(playlist_data->source);
}

uint32_t playlist_source_width(void *data)
{
	struct PlaylistSource *playlist_data = data;
	return obs_source_get_width(playlist_data->media_source);
}

uint32_t playlist_source_height(void *data)
{
	struct PlaylistSource *playlist_data = data;
	return obs_source_get_height(playlist_data->media_source);
}

void playlist_get_defaults(obs_data_t *settings)
{
	// obs_data_set_default_int(settings, "playlist_start_behavior", 0);
	// obs_data_set_default_int(settings, "playlist_end_behavior", 0);
	// obs_data_set_default_int(settings, "loop_index", 0);
	obs_data_set_default_bool(settings, "shuffle_queue", false);
	obs_data_set_default_bool(settings, "infinite", true);
	// obs_data_set_default_int(settings, "loop_count", 0);
	obs_data_set_default_bool(settings, "debug", false);
	obs_data_set_default_int(settings, "song_history_limit", 100);
	obs_data_set_default_int(settings, "queue_list_size", 5);
}

obs_properties_t *playlist_get_properties(void *data)
{
	return make_playlist_properties(data);
}

void playlist_update(void *data, obs_data_t *settings)
{
	update_playlist_data(data, settings);

	// Ensure properties are refreshed or updated if necessary
	// obs_properties_t *props = make_playlist_properties(playlist_data);
	// obs_source_update(playlist_data->source, props);
}

void playlist_activate(void *data)
{
	obs_log(LOG_INFO, "playlist_activate");
	struct PlaylistSource *playlist_data = data;

	switch (playlist_data->playlist_start_behavior) {
	case START_BEHAVIOR_RESTART_ENTIRE_PLAYLIST:
		obs_log(LOG_INFO, "We restarted the entire playlist");
		// playlist_data->current_media_index = 0;
		// playlist_data->current_media =
		// 	get_media(playlist_data->all_media, playlist_data->current_media_index);
		// playlist_data->current_media_source;
		// playlist_queue(playlist_data, playlist_data->current_media_index);

		// da_
		// obs_log_media_array(LOG_INFO, "Testing Queue 1: ", &playlist_data->all_media, 90, "    ", true);
		refresh_queue_list(playlist_data);

		// obs_log_media_array(LOG_INFO, "Testing Queue 2: ", &playlist_data->queue, 90, "    ", true);
		// obs_log_media_array(LOG_INFO, "Testing Queue 3: ", &playlist_data->all_media, 90, "    ", true);

		// obs_source_media_play_pause(playlist_data->source, false);
		// obs_source_media_stop(playlist_data->source);
		playlist_queue_restart(playlist_data);
		obs_source_update_properties(playlist_data->source);
		break;
	case START_BEHAVIOR_RESTART_AT_CURRENT_INEX:
		/* code */
		break;
	case START_BEHAVIOR_KEEP_SAME_BEHAVIOR:
		/* code */
		break;
	case START_BEHAVIOR_UNPAUSE:
		/* code */
		break;
	case START_BEHAVIOR_PAUSE:
		/* code */
		break;
	default:
		break;
	}
}

void playlist_deactivate(void *data)
{
	obs_log(LOG_INFO, "playlist_deactivate");
	struct PlaylistSource *playlist_data = data;
	// struct PlaylistSource *playlist_data = data;
	switch (playlist_data->playlist_start_behavior) {
	case START_BEHAVIOR_RESTART_ENTIRE_PLAYLIST:
		// obs_set_output_source(0, playlist_data->media_source);
		obs_source_media_ended(playlist_data->media_source);
		obs_source_media_stop(playlist_data->source);
		break;
	case START_BEHAVIOR_RESTART_AT_CURRENT_INEX:
		/* code */
		break;
	case START_BEHAVIOR_KEEP_SAME_BEHAVIOR:
		/* code */
		break;
	case START_BEHAVIOR_UNPAUSE:
		/* code */
		break;
	case START_BEHAVIOR_PAUSE:
		/* code */
		break;
	default:
		break;
	}
}

void playlist_video_tick(void *data, float seconds)
{
	struct PlaylistSource *playlist_data = data;
	//UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(seconds);

	const audio_t *a = obs_get_audio();
	const struct audio_output_info *aoi = audio_output_get_info(a);
	pthread_mutex_lock(&playlist_data->audio_mutex);
	while (playlist_data->audio_frames.size > 0) {
		struct obs_source_audio audio = {0};
		audio.format = aoi->format;
		audio.samples_per_sec = aoi->samples_per_sec;
		audio.speakers = aoi->speakers;

		deque_pop_front(&playlist_data->audio_frames, &audio.frames, sizeof(audio.frames));
		deque_pop_front(&playlist_data->audio_timestamps, &audio.timestamp, sizeof(audio.timestamp));

		for (size_t i = 0; i < playlist_data->num_channels; i++) {
			audio.data[i] =
				(uint8_t *)playlist_data->audio_data[i].data + playlist_data->audio_data[i].start_pos;
		}
		obs_source_output_audio(playlist_data->source, &audio);
		for (size_t i = 0; i < playlist_data->num_channels; i++) {
			deque_pop_front(&playlist_data->audio_data[i], NULL, audio.frames * sizeof(float));
		}
	}
	playlist_data->num_channels = audio_output_get_channels(a);
	// obs_log(LOG_INFO, "Channels: %d", playlist_data->num_channels);
	pthread_mutex_unlock(&playlist_data->audio_mutex);
}

void playlist_video_render(void *data, gs_effect_t *effect)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_video_render(playlist_data->media_source);
	} else {
		obs_source_video_render(NULL);
	}

	UNUSED_PARAMETER(effect);
	// obs_log(LOG_INFO, "video_render");
}

bool playlist_audio_render(void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio_output, uint32_t mixers,
			   size_t channels, size_t sample_rate)
{
	struct PlaylistSource *playlist_data = data;
	if (!playlist_data->media_source)
		return false;

	uint64_t source_ts;

	source_ts = obs_source_get_audio_timestamp(playlist_data->media_source);
	if (!source_ts)
		return false;

	// Get the audio from the child source (ffmpeg_source)
	struct obs_source_audio_mix child_audio_output = {0};

	// Retrieve the audio mix from the child source
	obs_source_get_audio_mix(playlist_data->media_source, &child_audio_output);

	// Iterate over all mixer outputs (for each mixer)
	for (size_t mix = 0; mix < MAX_AUDIO_MIXES; mix++) {
		// Skip if this mixer is not active in the 'mixers' bitmask
		if ((mixers & (1 << mix)) == 0)
			continue;

		// Iterate over all channels
		for (size_t ch = 0; ch < channels; ch++) {
			// Get pointers to the input and output buffers
			float *out = audio_output->output[mix].data[ch];
			float *in = child_audio_output.output[mix].data[ch];

			// Ensure the buffers are not null
			if (out && in) {
				// Use memcpy to copy the audio frames into the output buffer
				// Make sure we do not copy beyond the size of the buffer (use frames_per_channel)
				memcpy(out, in, AUDIO_OUTPUT_FRAMES * sizeof(float));
			}
		}
	}

	*ts_out = source_ts;

	UNUSED_PARAMETER(sample_rate);
	return true;
	// return false;
}

void playlist_enum_active_sources(void *data, obs_source_enum_proc_t enum_callback, void *param)
{
	struct PlaylistSource *playlist_data = data;

	pthread_mutex_lock(&playlist_data->mutex);
	enum_callback(playlist_data->source, playlist_data->media_source, param);
	pthread_mutex_unlock(&playlist_data->mutex);
}

void playlist_save(void *data, obs_data_t *settings)
{
	obs_log(LOG_INFO, "playlist_save");
	// struct PlaylistSource *playlist_data = data;
	// obs_data_set_int(settings, S_CURRENT_MEDIA_INDEX, playlist_data->current_media_index);
	// update_current_filename_setting(playlist_data, settings);
}

void playlist_load(void *data, obs_data_t *settings)
{
	obs_log(LOG_INFO, "playlist_load");
}

void media_play_pause(void *data, bool pause)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_log(LOG_INFO, "We be playing the video");
		obs_source_media_play_pause(playlist_data->media_source, pause);
	}
}

void media_restart(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_media_restart(playlist_data->media_source);
	}
}

void media_stop(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_media_stop(playlist_data->media_source);
	}
}

void media_next(void *data)
{
	struct PlaylistSource *playlist_data = data;

	pthread_mutex_lock(&playlist_data->mutex);

	if (playlist_data->queue.size > 0) {
		if (uses_song_history_limit(playlist_data) == true) {
			const MediaFileData *media_file_data = get_media(&playlist_data->queue, 0);

			const MediaFileData new_entry = create_media_file_data_with_all_info(
				media_file_data->path, media_file_data->filename, media_file_data->name,
				media_file_data->ext, media_file_data->index);

			push_media_file_data_front(&playlist_data->previous_queue, new_entry);

			if (playlist_data->previous_queue.size > playlist_data->song_history_limit) {
				pop_media_back(&playlist_data->previous_queue);
			}
		} else if (playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP) {
			const MediaFileData *media_file_data = get_media(&playlist_data->queue, 0);

			const MediaFileData new_entry = create_media_file_data_with_all_info(
				media_file_data->path, media_file_data->filename, media_file_data->name,
				media_file_data->ext, media_file_data->index);

			push_media_file_data_back(&playlist_data->queue, new_entry);
		}

		pop_media_front(&playlist_data->queue);
		playlist_queue_restart(playlist_data);
	}

	pthread_mutex_unlock(&playlist_data->mutex);

	obs_source_update_properties(playlist_data->source);
}

void media_previous(void *data)
{
	struct PlaylistSource *playlist_data = data;

	pthread_mutex_lock(&playlist_data->mutex);

	if (uses_song_history_limit(playlist_data) == true) {
		if (playlist_data->previous_queue.size > 0) {
			const MediaFileData *media_file_data = get_media(&playlist_data->previous_queue, 0);

			const MediaFileData new_entry = create_media_file_data_with_all_info(
				media_file_data->path, media_file_data->filename, media_file_data->name,
				media_file_data->ext, media_file_data->index);

			pop_media_front(&playlist_data->previous_queue);

			push_media_file_data_front(&playlist_data->queue, new_entry);

			playlist_queue_restart(playlist_data);
		}
	} else if (playlist_data->playlist_end_behavior == END_BEHAVIOR_LOOP) {
		obs_log(LOG_INFO, "Queue Size: %d", playlist_data->queue.size - 1);
		move_media_at(&playlist_data->queue, playlist_data->queue.size - 1, 0);
		playlist_queue_restart(playlist_data);
	}

	pthread_mutex_unlock(&playlist_data->mutex);
	obs_source_update_properties(playlist_data->source);
}

int64_t media_get_duration(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		return obs_source_media_get_duration(playlist_data->media_source);
	}
	return 0;
}

int64_t media_get_time(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		return obs_source_media_get_time(playlist_data->media_source);
	}
	return 0;
}

void media_set_time(void *data, int64_t miliseconds)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_media_set_time(playlist_data->media_source, miliseconds);
	}
}

enum obs_media_state media_get_state(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->all_media.size > 0 && playlist_data->media_source != NULL) {
		return obs_source_media_get_state(playlist_data->media_source);
	}
	return OBS_MEDIA_STATE_NONE;
}

#pragma endregion

#pragma endregion

/*
obs_encoder_t *obs_audio_encoder_create(const char *id, const char *name, obs_data_t *settings, size_t mixer_idx, obs_data_t *hotkey_data);
bool obs_audio_monitoring_available(void);
audio_t *obs_get_audio(void);
bool obs_get_audio_info(struct obs_audio_info *oai);
void obs_get_audio_monitoring_device(const char **name, const char **id);
bool obs_set_audio_monitoring_device(const char *name, const char *id);
void obs_enum_audio_monitoring_devices(obs_enum_audio_device_cb cb, void *data);
bool obs_reset_audio2(const struct obs_audio_info2 *oai);
bool obs_reset_audio(const struct obs_audio_info *oai);
void obs_reset_audio_monitoring(void);
audio_t *obs_output_audio(const obs_output_t *output);
bool obs_source_audio_active(const obs_source_t *source);
bool obs_source_audio_pending(const obs_source_t *source);
audio_t *obs_encoder_audio(const obs_encoder_t *encoder);
void obs_add_raw_audio_callback(size_t mix_idx, const struct audio_convert_info *conversion, audio_output_callback_t callback, void *param);
obs_encoder_t *obs_output_get_audio_encoder(const obs_output_t *output, size_t idx);
void obs_output_set_audio_conversion(obs_output_t *output, const struct audio_convert_info *conversion);
void obs_output_set_audio_encoder(obs_output_t *output, obs_encoder_t *encoder, size_t idx);
void obs_remove_raw_audio_callback(size_t mix_idx, audio_output_callback_t callback, void *param);
void obs_source_add_audio_capture_callback(obs_source_t *source, obs_source_audio_capture_t callback, void *param);
void obs_source_add_audio_pause_callback(obs_source_t *source, signal_callback_t callback, void *param);
void obs_source_get_audio_mix(const obs_source_t *source, struct obs_source_audio_mix *audio);
uint32_t obs_source_get_audio_mixers(const obs_source_t *source);
uint64_t obs_source_get_audio_timestamp(const obs_source_t *source);
void obs_source_set_audio_active(obs_source_t *source, bool show);
void obs_source_set_audio_mixers(obs_source_t *source, uint32_t mixers);
bool obs_transition_audio_render(obs_source_t *transition, uint64_t *ts_out, struct obs_source_audio_mix *audio, uint32_t mixers, size_t channels, size_t sample_rate, obs_transition_audio_mix_callback_t mix_a_callback, obs_transition_audio_mix_callback_t mix_b_callback);
void obs_encoder_set_audio(obs_encoder_t *encoder, audio_t *audio);
void obs_hotkeys_set_audio_hotkeys_translations(const char *mute, const char *unmute, const char *push_to_mute, const char *push_to_talk);
void obs_source_output_audio(obs_source_t *source, const struct obs_source_audio *audio);
void obs_source_remove_audio_capture_callback(obs_source_t *source, obs_source_audio_capture_t callback, void *param);
void obs_source_remove_audio_pause_callback(obs_source_t *source, signal_callback_t callback, void *param);
const char *obs_get_output_supported_audio_codecs(const char *id);
const char *obs_output_get_supported_audio_codecs(const obs_output_t *output);
const char **obs_service_get_supported_audio_codecs(const obs_service_t *service);
*/

/*
scene
group
audio_line
image_source
color_source
color_source_v2
color_source_v3
slideshow
slideshow_v2
browser_source
ffmpeg_source
mask_filter
mask_filter_v2
crop_filter
gain_filter
basic_eq_filter
hdr_tonemap_filter
color_filter
color_filter_v2
scale_filter
scroll_filter
gpu_delay
color_key_filter
color_key_filter_v2
clut_filter
sharpness_filter
sharpness_filter_v2
chroma_key_filter
chroma_key_filter_v2
async_delay_filter
noise_suppress_filter
noise_suppress_filter_v2
invert_polarity_filter
noise_gate_filter
compressor_filter
limiter_filter
expander_filter
upward_compressor_filter
luma_key_filter
luma_key_filter_v2
text_gdiplus
text_gdiplus_v2
text_gdiplus_v3
cut_transition
fade_transition
swipe_transition
slide_transition
obs_stinger_transition
fade_to_color_transition
wipe_transition
vst_filter
text_ft2_source
text_ft2_source_v2
monitor_capture
window_capture
game_capture
dshow_input
wasapi_input_capture
wasapi_output_capture
wasapi_process_output_capture
*/

/*

*/