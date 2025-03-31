#pragma region Main

#include "../include/sources/playlist-source.hpp"

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

void refresh_queue_list(PlaylistData *playlist_data)
{
	PlaylistContext *playlist_context = playlist_data->playlist_context;
	size_t all_size = playlist_context->all_media.size();
	size_t queue_size = playlist_context->queue.size();
	size_t max_size = std::max(all_size, queue_size);

	for (size_t i = 0; i < max_size; i++) {
		if (i < queue_size) {
			pop_queue_media_at(&playlist_context->queue, queue_size - i - 1); // Remove in reverse order
		}
		if (i < all_size) {
			push_queue_media_data_back(&playlist_context->queue, playlist_context->all_media[i],
						   playlist_data);
		}
	}
}

void playlist_media_source_ended(void *data, const char *signal, calldata_t *callback)
{

	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;
	if (strcmp(signal, "media_ended") == 0) {
		obs_log(LOG_INFO, "Signal: %s", signal);

		UNUSED_PARAMETER(callback);
		playlist_context->state = OBS_MEDIA_STATE_ENDED;

		obs_source_media_next(playlist_context->source);
	} else if (strcmp(signal, "media_restart") == 0) {
		obs_log(LOG_INFO, "Signal: %s", signal);
	}
}

void set_queue(PlaylistContext *playlist_context)
{
	if (!playlist_context || !playlist_context->media_source || !playlist_context->media_source_settings)
		return;

	if (playlist_context->queue.size() <= 0) {
		obs_source_media_stop(playlist_context->source);
		// obs_log(LOG_INFO, "WE STOPPED IT YAY!!!");
		return;
	}
	if (playlist_context->all_media.size() <= 0) {
		return;
	}
	// obs_log(LOG_INFO, "WE TRIED SET THE QUEUE YAY!!!");

	// Get video file path from the array
	MediaData *media_data = &playlist_context->queue[0]->media_data;

	if (!media_data)
		return;
	// obs_log(LOG_INFO, "WE SET THE QUEUE YAY!!!");

	obs_data_set_string(playlist_context->media_source_settings, S_FFMPEG_LOCAL_FILE, media_data->path.c_str());
	obs_source_update(playlist_context->media_source, playlist_context->media_source_settings);
}

void clear_any_media_playing(PlaylistContext *playlist_context)
{
	if (!playlist_context->media_source || !playlist_context->media_source_settings)
		return;

	// Set file path in ffmpeg_source
	obs_data_set_string(playlist_context->media_source_settings, S_FFMPEG_LOCAL_FILE, "");

	// obs_log(LOG_INFO, " PRINTING: /n%s", obs_data_get_json_pretty(playlist_context->media_source_settings));

	obs_source_update(playlist_context->media_source, playlist_context->media_source_settings);

	obs_source_media_stop(playlist_context->source);
}

void playlist_audio_callback(void *data, obs_source_t *source, const struct audio_data *audio_data, bool muted)
{
	UNUSED_PARAMETER(muted);
	UNUSED_PARAMETER(source);

	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	pthread_mutex_lock(&playlist_context->audio_mutex);

	size_t size = audio_data->frames * sizeof(float);
	for (size_t i = 0; i < playlist_context->num_channels; i++) {
		deque_push_back(&playlist_context->audio_data[i], audio_data->data[i], size);
	}
	deque_push_back(&playlist_context->audio_frames, &audio_data->frames, sizeof(audio_data->frames));
	deque_push_back(&playlist_context->audio_timestamps, &audio_data->timestamp, sizeof(audio_data->timestamp));

	pthread_mutex_unlock(&playlist_context->audio_mutex);
}

bool uses_song_history_limit(PlaylistContext *playlist_context)
{
	return ((playlist_context->end_behavior == END_BEHAVIOR_LOOP_AT_END &&
		 playlist_context->shuffle_queue == true) ||
		(playlist_context->end_behavior == END_BEHAVIOR_LOOP && playlist_context->shuffle_queue == true) ||
		(playlist_context->end_behavior == END_BEHAVIOR_STOP && playlist_context->shuffle_queue == true));
}
#pragma endregion

#pragma region Property Managment

void show_param_queue(PlaylistData *playlist_data)
{
	if (playlist_data->param_playlist_widget) {
		QWidget *obs_main_window = (QWidget *)obs_frontend_get_main_window();
		QWidget *properties_window = nullptr;

		for (QObject *child : obs_main_window->children()) {
			QWidget *widget = qobject_cast<QWidget *>(child);
			if (widget && widget->objectName() == "OBSBasicProperties" &&
			    widget->windowTitle() ==
				    QString::fromStdString("Properties for '" + playlist_data->playlist_context->name +
							   "'")) {
				properties_window = widget;
				break;
			}
		}

		if (properties_window) {
			playlist_data->param_playlist_widget->setParent(properties_window, Qt::Window);
			QObject::connect(properties_window, &QObject::destroyed, playlist_data->param_playlist_widget,
					 [playlist_data]() {
						 playlist_data->param_playlist_widget->setParent(nullptr);
						 playlist_data->param_playlist_widget->hide();
					 });
		} else {
			playlist_data->param_playlist_widget->setParent(obs_main_window, Qt::Window);
		}

		playlist_data->param_playlist_widget->setWindowFlags(Qt::Tool);

		playlist_data->param_playlist_widget->show();
	}
}

bool show_param_queue_button(obs_properties_t *props, obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	show_param_queue(playlist_data);
	return true;
}

obs_properties_t *make_playlist_properties(PlaylistData *playlist_data)
{
	PlaylistContext *playlist_context = playlist_data->playlist_context;
	obs_properties_t *props = obs_properties_create();

	obs_properties_add_bool(props, "show_queue_when_properties_open", "Show Queue When Properties Open");
	obs_properties_add_button(props, "show_queue", "Show Queue", show_param_queue_button);

	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");

	obs_properties_add_bool(props, "shuffle_queue", "Shuffle Queue");

	obs_property_t *psb_property = obs_properties_add_list(props, "start_behavior", "Playlist Start Behavior",
							       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	add_enums_to_property_list(psb_property, StartBehavior, 2);

	obs_property_t *peb_property = obs_properties_add_list(props, "end_behavior", "Playlist End Behavior",
							       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	add_enums_to_property_list(peb_property, EndBehavior, 2);

	if (playlist_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
		obs_properties_add_bool(props, "infinite", "Infinite");
		if (playlist_context->infinite == false) {
			obs_properties_add_int(props, "max_loop_count", "Loop Count", 0, INT_MAX, 1);

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
void update_playlist_data(PlaylistData *playlist_data, obs_data_t *settings)
{
	PlaylistContext *playlist_context = playlist_data->playlist_context;
	playlist_context->show_queue_when_properties_open =
		obs_data_get_bool(settings, "show_queue_when_properties_open");

	bool update_properties = false;

	pthread_mutex_lock(&playlist_context->mutex);

	obs_data_array_t *obs_playlist = obs_data_get_array(settings, "playlist");

	size_t array_size = obs_data_array_count(obs_playlist);
	size_t pre_media_array_size = playlist_context->all_media.size();

	if (array_size <= 0) {
		playlist_context->all_media.clear();
		if (playlist_context->queue.size() > 0) {
			clear_queue(&playlist_context->queue);
			set_queue(playlist_context);
		}
	} else {
		size_t entry_index = 0;
		MediaDataArray removed_medias{};
		MediaDataArray added_medias{};
		for (size_t i = 0; i < array_size; ++i) {
			obs_data_t *data = obs_data_array_item(obs_playlist, i);

			if (data == NULL) {
				continue; // Skip if data is NULL (avoid potential crash or issues)
			}

			const char *element = obs_data_get_string(data, "value");
			if (element == NULL) {
				obs_data_release(data); // Release memory for the current element before skipping
				continue;               // Skip if no valid string was found
			}

			// obs_log(LOG_INFO, "Found El Path: %s", element);
			fs::path file_path = element;

			if (fs::is_directory(file_path)) {
				for (const fs::directory_entry &entry : fs::recursive_directory_iterator(file_path)) {
					// Print the path of each entry (file or directory)
					if (entry.is_directory())
						continue;

					fs::path entry_path = entry.path();

					// Get the extension of the file or directory
					std::string extension = entry_path.extension().string();

					if (!valid_extension(&extension))
						continue;

					MediaData new_entry =
						init_media_data_from_path(entry.path().string(), entry_index);
					if (entry_index < playlist_context->all_media.size()) {
						const MediaData media_data = playlist_context->all_media[i];
						if (media_data.path != new_entry.path) {
							if (pre_media_array_size > array_size) {
								removed_medias.push_back(media_data);
							}
							playlist_context->all_media[entry_index] = new_entry;
						}
					} else {
						push_media_data_at(&playlist_context->all_media, new_entry,
								   entry_index);
						added_medias.push_back(new_entry);
					}
					entry_index++;
				}
			} else {
				MediaData new_entry = init_media_data_from_path(std::string(element), entry_index);
				if (entry_index < playlist_context->all_media.size()) {
					const MediaData media_data = playlist_context->all_media[i];
					if (media_data.path != new_entry.path) {
						if (pre_media_array_size > array_size) {
							removed_medias.push_back(media_data);
						}
						playlist_context->all_media[entry_index] = new_entry;
					}
				} else {
					push_media_data_at(&playlist_context->all_media, new_entry, entry_index);
					// obs_log(LOG_INFO, "Media Entry: %s, %d", new_entry.path.c_str(),
					// 	new_entry.index);
					added_medias.push_back(new_entry);
				}
				entry_index++;
			}
			obs_data_release(data);
		}

		for (size_t i = playlist_context->all_media.size(); i-- > entry_index;) {
			MediaData new_entry = playlist_context->all_media[i];
			if (pre_media_array_size > array_size) {
				if (removed_medias.size() == 0) {
					removed_medias.push_front(new_entry);
				}
			}
			pop_media_at(&playlist_context->all_media, i);
		}

		if (playlist_context->all_media_initialized == true) {
			bool changed_queue = false;
			for (size_t i = playlist_context->queue.size(); i-- > 0;) {
				SharedQueueMediaData queue_media_data = playlist_context->queue[i];
				if (queue_media_data->media_data.index < playlist_context->all_media.size()) {
					MediaData media_data =
						playlist_context->all_media[queue_media_data->media_data.index];

					if (queue_media_data->media_data.path != media_data.path) {
						SharedQueueMediaData new_entry = init_queue_media_data_from_media_data(
							media_data, i, playlist_data, queue_media_data->media_widget,
							queue_media_data->param_media_widget);

						playlist_context->queue[i] = new_entry;

						new_entry->media_widget->update_media_data();
						new_entry->param_media_widget->update_media_data();

						if (changed_queue == false)
							changed_queue = true;
					}
				}
			}
			for (size_t i = playlist_context->queue.size(); i-- > 0;) {
				SharedQueueMediaData queue_media_data = playlist_context->queue[i];
				for (size_t r_i = removed_medias.size(); r_i-- > 0;) {
					MediaData removed_media_data = removed_medias[r_i];
					if (queue_media_data->media_data.index == removed_media_data.index) {
						pop_queue_media_at(&playlist_context->queue, i, true);
						if (changed_queue == false)
							changed_queue = true;
					}
				}
			}
			bool found_queue = false;
			size_t queue_last_index = 0;

			if (playlist_context->queue.size() > 0) {
				queue_last_index =
					playlist_context->queue[playlist_context->queue.size() - 1]->media_data.index;
				found_queue = true;
			}

			// to-do Only add songs that have actually been added to the all media list.
			for (size_t i = 0; i < added_medias.size(); i++) {
				// obs_log(LOG_INFO, "Queue Index: %d", i);
				MediaData added_media_data = added_medias[i];
				if (added_media_data.index > queue_last_index || found_queue == false) {
					push_queue_media_data_back(&playlist_context->queue, added_media_data,
								   playlist_data);
					if (changed_queue == false)
						changed_queue = true;
				}
			}
			if (changed_queue == true) {
				set_queue(playlist_context);
				if (found_queue == false) {
					obs_source_media_restart(playlist_context->media_source);
				}
			}
		}
	}

	obs_data_array_release(obs_playlist);

	if (playlist_context->all_media_initialized == false) {
		playlist_context->all_media_initialized = true;
	}

	pthread_mutex_unlock(&playlist_context->mutex);

	if (playlist_context->debug == true) {
		obs_log_media_array(LOG_INFO, "All Media:/n", &playlist_context->all_media, 0, "    ",
				    MEDIA_STRINGIFY_TYPE_FILENAME);
	}

	bool is_queue_shuffle = obs_data_get_bool(settings, "shuffle_queue");
	bool shuffle_changed = is_queue_shuffle != playlist_context->shuffle_queue;

	if (shuffle_changed)
		playlist_context->shuffle_queue = is_queue_shuffle;

	playlist_context->start_behavior = (e_StartBehavior)obs_data_get_int(settings, "start_behavior");
	if (playlist_context->debug == true) {
		obs_log(LOG_INFO, "Start Behavior: %s", StartBehavior[playlist_context->start_behavior]);
	}

	e_EndBehavior end_behavior = (e_EndBehavior)obs_data_get_int(settings, "end_behavior");

	bool end_behavior_changed = playlist_context->end_behavior != end_behavior;

	if (end_behavior_changed) {
		update_properties = true;
	}

	playlist_context->end_behavior = end_behavior;
	if (playlist_context->debug == true) {
		obs_log(LOG_INFO, "End Behavior: %s", EndBehavior[playlist_context->end_behavior]);
	}

	pthread_mutex_lock(&playlist_context->mutex);

	switch (playlist_context->end_behavior) {
	case END_BEHAVIOR_STOP:
		if (is_queue_shuffle == false) {
			// if (end_behavior_changed == true) {
			// 	obs_source_media_restart(playlist_context->source);
			// }
			// } else {
			if (end_behavior_changed == true || shuffle_changed == true) {
				obs_log(LOG_INFO, "This should restart");
				obs_source_media_restart(playlist_context->source);
			} else if (playlist_context->queue.size() > 0) {
				SharedQueueMediaData first_queue_media = playlist_context->queue[0];
				for (size_t i = playlist_context->queue.size(); i-- > 0;) {
					SharedQueueMediaData queue_media_data = playlist_context->queue[i];
					if (queue_media_data->media_data.index < first_queue_media->media_data.index) {
						pop_queue_media_at(&playlist_context->queue, i);
					}
				}
			}
		} else {
			if (end_behavior_changed == true || shuffle_changed == true) {
				obs_log(LOG_INFO, "This should restart");
				obs_source_media_restart(playlist_context->source);
			}
		}
		break;
	case END_BEHAVIOR_LOOP:
		if (is_queue_shuffle == false) {
			// } else {
			if (shuffle_changed == true) {
				obs_source_media_restart(playlist_context->source);
			} else if (playlist_context->queue.size() < playlist_context->all_media.size()) {
				if (playlist_context->queue.size() > 0) {
					SharedQueueMediaData first_queue_media_data = playlist_context->queue[0];

					size_t queue_index = 1;

					for (size_t i = first_queue_media_data->media_data.index + 1;
					     i < playlist_context->all_media.size(); i++) {
						MediaData media_data = playlist_context->all_media[i];
						if (queue_index < playlist_context->queue.size()) {
							SharedQueueMediaData queue_media_data =
								playlist_context->queue[queue_index];

							if (queue_media_data->media_data.path != media_data.path) {
								SharedQueueMediaData new_entry =
									init_queue_media_data_from_media_data(
										media_data, i, playlist_data,
										queue_media_data->media_widget,
										queue_media_data->param_media_widget);

								playlist_context->queue[queue_index] = new_entry;

								new_entry->media_widget->update_media_data();
								new_entry->param_media_widget->update_media_data();
							}
						} else {
							push_queue_media_data_at(&playlist_context->queue, media_data,
										 queue_index, playlist_data);
						}
						queue_index++;
					}
					for (size_t i = 0; i < first_queue_media_data->media_data.index; i++) {
						MediaData media_data = playlist_context->all_media[i];
						if (queue_index < playlist_context->queue.size()) {
							SharedQueueMediaData queue_media_data =
								playlist_context->queue[queue_index];

							if (queue_media_data->media_data.path != media_data.path) {
								SharedQueueMediaData new_entry =
									init_queue_media_data_from_media_data(
										media_data, i, playlist_data,
										queue_media_data->media_widget,
										queue_media_data->param_media_widget);

								playlist_context->queue[queue_index] = new_entry;

								new_entry->media_widget->update_media_data();
								new_entry->param_media_widget->update_media_data();
							}
						} else {
							push_queue_media_data_at(&playlist_context->queue, media_data,
										 queue_index, playlist_data);
						}
						queue_index++;
					}
				} else {
					obs_source_media_restart(playlist_context->source);
				}
			}
		}
		break;
	case END_BEHAVIOR_LOOP_AT_END:
		break;
	default:
		break;
	}

	pthread_mutex_unlock(&playlist_context->mutex);

	int all_media_size = (int)playlist_context->all_media.size();

	int last_index = all_media_size - 1;

	if (last_index < 0) {
		last_index += 1;
	}

	if (playlist_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
		bool infinite = obs_data_get_bool(settings, "infinite");
		if (infinite != playlist_context->infinite) {
			update_properties = true;
		}
		playlist_context->infinite = infinite;
		if (playlist_context->infinite) {
			playlist_context->max_loop_count = (int)obs_data_get_int(settings, "max_loop_count");
			playlist_context->loop_end_behavior =
				(e_LoopEndBehavior)obs_data_get_int(settings, "loop_end_behavior");
		}
		if (playlist_data->playlist_context->debug == true) {
			obs_log(LOG_INFO, "Infinite: %s", playlist_context->infinite == true ? "true" : "false");
			if (playlist_context->infinite == false && playlist_context->debug == true) {
				obs_log(LOG_INFO, "Loop Count: %d", playlist_context->max_loop_count);
			}
		}
	}

	playlist_context->song_history_limit = (int)obs_data_get_int(settings, "song_history_limit");

	playlist_context->debug = obs_data_get_bool(settings, "debug");
	if (playlist_context->debug == true) {
		obs_log(LOG_INFO, "Debug: %s", playlist_context->debug ? "true" : "false");
	}

	if (update_properties == true) {
		obs_source_update_properties(playlist_context->source);
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
	PlaylistData *playlist_data = new PlaylistData();
	PlaylistContext *playlist_context = new PlaylistContext();
	playlist_data->playlist_context = playlist_context;

	std::stringstream ss;
	ss << obs_source_get_name(source);
	playlist_context->name = ss.str();

	playlist_context->source = source;

	playlist_context->media_source_settings = obs_data_create();

	obs_data_set_bool(playlist_context->media_source_settings, S_FFMPEG_LOG_CHANGES, false);
	obs_data_set_bool(playlist_context->media_source_settings, S_FFMPEG_RESTART_PLAYBACK_ON_ACTIVE, false);
	playlist_context->media_source =
		obs_source_create_private("ffmpeg_source", "Video Source", playlist_context->media_source_settings);

	obs_source_add_active_child(playlist_context->source, playlist_context->media_source);
	obs_source_add_audio_capture_callback(playlist_context->media_source, playlist_audio_callback, playlist_data);

	// signal_handler_t *sh_source = obs_source_get_signal_handler(playlist_context->source);
	// signal_handler_connect_global(sh_source, playlist_source_callbacks, playlist_context);

	signal_handler_t *sh_media_source = obs_source_get_signal_handler(playlist_context->media_source);
	// signal_handler_connect(sh_media_source, "media_ended", playlist_media_source_ended, playlist_data);
	signal_handler_connect_global(sh_media_source, playlist_media_source_ended, playlist_data);

	playlist_context->show_queue_when_properties_open = true;

	playlist_context->all_media_initialized = false;

	playlist_context->all_media = {};

	playlist_context->queue = {};

	playlist_context->queue_history = {};

	// playlist_context->current_media = NULL;
	// playlist_context->current_media_index = 0;
	playlist_context->state = OBS_MEDIA_STATE_NONE;
	playlist_context->num_channels = 2;
	playlist_context->loop_index = 0;
	playlist_context->infinite = true;
	playlist_context->max_loop_count = 0;
	playlist_context->loop_count = 0;
	playlist_context->song_history_limit = 100;

	// playlist_context->properties = make_playlist_properties(playlist_context);

	// QWidget *properties_window = (QWidget *)obs_frontend_get_main_window();
	// playlist_context->properties_ui = new CustomProperties(settings, properties_window);

	// pthread_mutex_init_value(&playlist_context->mutex);
	// QWidget *obs_main_window = (QWidget *)obs_frontend_get_main_window();
	if (pthread_mutex_init(&playlist_context->mutex, NULL) != 0) {
		goto error;
	}

	// pthread_mutex_init_value(&playlist_context->audio_mutex);
	if (pthread_mutex_init(&playlist_context->audio_mutex, NULL) != 0) {
		goto error;
	}

	update_playlist_data(playlist_data, settings);

	playlist_data->param_playlist_widget = new PlaylisQueueWidget(playlist_context, nullptr, true);
	playlist_data->param_playlist_widget->setObjectName("playlist_queue_viewer_maximon9");

	playlist_data->playlist_widget = new PlaylisQueueWidget(playlist_context, multi_playlist_queue_viewer, false);
	multi_playlist_queue_viewer->addPlaylistWidget(playlist_data->playlist_widget);
	multi_playlist_queue_viewer->playlist_datas.push_back(playlist_data);

	return playlist_data;
error:
	playlist_source_destroy(playlist_data);
	return NULL;
}

void playlist_source_destroy(void *data)
{
	// obs_log(LOG_INFO, "Destroying Playlist");
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	if (playlist_context->media_source != NULL) {
		obs_source_release(playlist_context->media_source);
	}
	if (playlist_context->media_source_settings != NULL) {
		obs_data_release(playlist_context->media_source_settings);
	}

	for (size_t i = 0; i < MAX_AUDIO_CHANNELS; i++) {
		deque_free(&playlist_context->audio_data[i]);
	}
	deque_free(&playlist_context->audio_frames);
	deque_free(&playlist_context->audio_timestamps);
	pthread_mutex_destroy(&playlist_context->mutex);
	pthread_mutex_destroy(&playlist_context->audio_mutex);

	if (playlist_data->playlist_widget != nullptr) {
		playlist_data->playlist_widget->remove_widget();
		delete playlist_data->playlist_widget;
	}
	if (playlist_data->param_playlist_widget != nullptr) {
		playlist_data->param_playlist_widget->remove_widget();
		delete playlist_data->param_playlist_widget;
	}

	// free_media_array(&playlist_context->queue);
	// free_media_array(&playlist_context->queue_history);

	// if (playlist_context->current_media != NULL) {
	// 	free(playlist_context->current_media);
	// }

	delete playlist_data;
	delete playlist_context;
	obs_source_release(playlist_context->source);
}

uint32_t playlist_source_width(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	return obs_source_get_width(playlist_context->media_source);
}

uint32_t playlist_source_height(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	return obs_source_get_height(playlist_context->media_source);
}

void playlist_get_defaults(obs_data_t *settings)
{
	// obs_data_set_default_int(settings, "start_behavior", 0);
	// obs_data_set_default_int(settings, "end_behavior", 0);
	// obs_data_set_default_int(settings, "loop_index", 0);
	obs_data_set_default_bool(settings, "show_queue_when_properties_open", true);
	obs_data_set_default_bool(settings, "shuffle_queue", false);
	obs_data_set_default_bool(settings, "infinite", true);
	obs_data_set_default_int(settings, "max_loop_count", 0);
	obs_data_set_default_bool(settings, "debug", false);
	obs_data_set_default_int(settings, "song_history_limit", 100);
}

obs_properties_t *playlist_get_properties(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	if (playlist_context->show_queue_when_properties_open == true) {
		show_param_queue(playlist_data);
	}

	return make_playlist_properties(playlist_data);
}

void playlist_update(void *data, obs_data_t *settings)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);

	update_playlist_data(playlist_data, settings);
}

void playlist_activate(void *data)
{
	obs_log(LOG_INFO, "playlist_activate");
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	switch (playlist_context->start_behavior) {
	case START_BEHAVIOR_RESTART_ENTIRE_PLAYLIST:
		obs_source_media_restart(playlist_context->source);
		obs_source_media_started(playlist_context->source);
		break;
	case START_BEHAVIOR_RESTART_AT_CURRENT_INDEX:
		set_queue(playlist_context);
		obs_source_media_restart(playlist_context->media_source);
		obs_source_media_started(playlist_context->source);
		break;
	case START_BEHAVIOR_UNPAUSE:
		set_queue(playlist_context);
		obs_source_media_play_pause(playlist_context->source, false);
		break;
	case START_BEHAVIOR_PAUSE:
		obs_source_media_play_pause(playlist_context->source, true);
		break;
	default:
		break;
	}
	// obs_source_update_properties(playlist_data->playlist_context->source);
}

void playlist_deactivate(void *data)
{
	obs_log(LOG_INFO, "playlist_deactivate");
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	switch (playlist_context->start_behavior) {
	case START_BEHAVIOR_RESTART_ENTIRE_PLAYLIST:
		obs_source_media_stop(playlist_context->source);
		obs_source_media_ended(playlist_context->source);
		break;
	case START_BEHAVIOR_RESTART_AT_CURRENT_INDEX:
		obs_source_media_stop(playlist_context->media_source);
		obs_source_media_ended(playlist_context->source);
		break;
	case START_BEHAVIOR_UNPAUSE:
		obs_source_media_play_pause(playlist_context->source, true);
		break;
	case START_BEHAVIOR_PAUSE:
		obs_source_media_play_pause(playlist_context->source, true);
		break;
	default:
		break;
	}
}

void playlist_video_tick(void *data, float seconds)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	//UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(seconds);

	const audio_t *a = obs_get_audio();
	const struct audio_output_info *aoi = audio_output_get_info(a);

	pthread_mutex_lock(&playlist_context->audio_mutex);

	while (playlist_context->audio_frames.size > 0) {
		struct obs_source_audio audio = {0};
		audio.format = aoi->format;
		audio.samples_per_sec = aoi->samples_per_sec;
		audio.speakers = aoi->speakers;

		deque_pop_front(&playlist_context->audio_frames, &audio.frames, sizeof(audio.frames));
		deque_pop_front(&playlist_context->audio_timestamps, &audio.timestamp, sizeof(audio.timestamp));

		for (size_t i = 0; i < playlist_context->num_channels; i++) {
			audio.data[i] = (uint8_t *)playlist_context->audio_data[i].data +
					playlist_context->audio_data[i].start_pos;
		}
		obs_source_output_audio(playlist_context->source, &audio);
		for (size_t i = 0; i < playlist_context->num_channels; i++) {
			deque_pop_front(&playlist_context->audio_data[i], NULL, audio.frames * sizeof(float));
		}
	}

	playlist_context->num_channels = audio_output_get_channels(a);

	pthread_mutex_unlock(&playlist_context->audio_mutex);
}

void playlist_video_render(void *data, gs_effect_t *effect)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	if (playlist_context->media_source != NULL) {
		obs_source_video_render(playlist_context->media_source);
	} else {
		obs_source_video_render(NULL);
	}

	UNUSED_PARAMETER(effect);
	// obs_log(LOG_INFO, "video_render");
}

bool playlist_audio_render(void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio_output, uint32_t mixers,
			   size_t channels, size_t sample_rate)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	if (!playlist_context->media_source)
		return false;

	uint64_t source_ts = obs_source_get_audio_timestamp(playlist_context->media_source);
	if (!source_ts)
		return false;

	// Get the audio from the child source (ffmpeg_source)
	struct obs_source_audio_mix child_audio_output = {0};

	// Retrieve the audio mix from the child source
	obs_source_get_audio_mix(playlist_context->media_source, &child_audio_output);

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

/* obs_properties_t *playlist_get_properties2(void *data, void *type_data)
{
	obs_log(LOG_INFO, "Yo wassup");

	return nullptr;
} */

void playlist_enum_active_sources(void *data, obs_source_enum_proc_t enum_callback, void *param)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	pthread_mutex_lock(&playlist_context->mutex);
	enum_callback(playlist_context->source, playlist_context->media_source, param);
	pthread_mutex_unlock(&playlist_context->mutex);
}

void playlist_save(void *data, obs_data_t *settings)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	obs_log(LOG_INFO, "playlist_save");

	playlist_context->name = obs_source_get_name(playlist_context->source);
	if (playlist_data->playlist_widget != nullptr) {
		playlist_data->playlist_widget->update_playlist_name();
	}
	if (playlist_data->param_playlist_widget != nullptr) {
		playlist_data->param_playlist_widget->update_playlist_name();
	}

	// 	if (playlist_data->param_playlist_widget) {
	// 		playlist_data->param_playlist_widget->hide();
	// 	}
}

void playlist_load(void *data, obs_data_t *settings)
{
	obs_log(LOG_INFO, "playlist_load");
}

void media_play_pause(void *data, bool pause)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	// pthread_mutex_lock(&playlist_context->mutex);

	if (playlist_context->media_source != NULL) {
		if (pause == true) {
			playlist_context->state = OBS_MEDIA_STATE_PAUSED;
		} else {
			playlist_context->state = OBS_MEDIA_STATE_PLAYING;
		}
		obs_source_media_play_pause(playlist_context->media_source, pause);
	}

	// pthread_mutex_unlock(&playlist_context->mutex);
}

void media_restart(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	// pthread_mutex_lock(&playlist_context->mutex);

	if (playlist_context->media_source != NULL) {
		playlist_context->loop_count = 0;

		refresh_queue_list(playlist_data);

		if (playlist_context->shuffle_queue == true) {
			shuffle_queue(&playlist_context->queue, playlist_data);
			playlist_context->queue_history.clear();
		}

		set_queue(playlist_context);

		if (playlist_context->queue.size() > 0) {
			playlist_context->state = OBS_MEDIA_STATE_PLAYING;
			obs_source_media_restart(playlist_context->media_source);
		}
	}

	// pthread_mutex_unlock(&playlist_context->mutex);
}

void media_stop(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	// pthread_mutex_lock(&playlist_context->mutex);

	if (playlist_context->media_source != NULL) {
		playlist_context->state = OBS_MEDIA_STATE_STOPPED;
		clear_queue(&playlist_context->queue);
		obs_source_media_stop(playlist_context->media_source);
	}

	// pthread_mutex_unlock(&playlist_context->mutex);
}

void media_next(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	pthread_mutex_lock(&playlist_context->mutex);

	if (playlist_context->queue.size() > 0) {
		if (uses_song_history_limit(playlist_context) == true) {
			bool add_to_history = false;
			if (playlist_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
				add_to_history = playlist_context->queue.size() > 1;
			} else {
				add_to_history = true;
			}
			if (add_to_history == true) {
				const MediaData media_data = playlist_context->queue[0]->media_data;

				push_media_data_front(&playlist_context->queue_history, media_data);

				if (playlist_context->queue_history.size() > playlist_context->song_history_limit) {
					playlist_context->queue_history.pop_back();
				}
			}

			switch (playlist_context->end_behavior) {
			case END_BEHAVIOR_LOOP:
				pop_queue_media_front(&playlist_context->queue);
				if (playlist_context->all_media.size() > 0) {
					if (playlist_context->queue.size() < playlist_context->all_media.size()) {
						MediaData random_media_data =
							playlist_context->all_media[get_random_size_t(
								0, playlist_context->all_media.size() - 1)];
						push_queue_media_data_back(&playlist_context->queue, random_media_data,
									   playlist_data);
					}
				}
				break;
			case END_BEHAVIOR_LOOP_AT_END:
				if (playlist_context->queue.size() > 1) {
					pop_queue_media_front(&playlist_context->queue);
					break;
				}
				if (playlist_context->infinite == false && playlist_context->queue.size() == 1) {
					playlist_context->loop_count++;
				}
			default:
				pop_queue_media_front(&playlist_context->queue);
				break;
			}
		} else if (playlist_context->end_behavior == END_BEHAVIOR_LOOP) {
			if (playlist_context->queue.size() > 1) {
				SharedQueueMediaData new_entry = pop_queue_media_front(&playlist_context->queue);
				if (playlist_context->queue.size() > 0) {
					init_widgets(new_entry, playlist_context->queue.size(), playlist_data);
				}
				playlist_context->queue.push_back(new_entry);
			}
		} else if (playlist_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
			if (playlist_context->queue.size() > 1) {
				pop_queue_media_front(&playlist_context->queue);
			}
			if (playlist_context->infinite == false && playlist_context->queue.size() == 1) {
				playlist_context->loop_count++;
			}
		} else {
			pop_queue_media_front(&playlist_context->queue);
		}

		if (playlist_context->queue.size() <= 0) {
			obs_source_media_stop(playlist_context->media_source);
			obs_source_media_ended(playlist_context->source);
		} else {
			if (playlist_context->loop_count <= playlist_context->max_loop_count) {
				bool restart = playlist_context->queue[0]->media_data.path ==
					       get_current_media_input(playlist_context->media_source_settings);

				set_queue(playlist_context);

				if (restart) {
					obs_source_media_restart(playlist_context->media_source);
				}
			}
		}
	} else {
		obs_source_media_stop(playlist_context->media_source);
		obs_source_media_ended(playlist_context->source);
	}

	// obs_log(LOG_INFO, "Buffer Offest: %f", obs_source_get_sync_offset(playlist_context->source));

	pthread_mutex_unlock(&playlist_context->mutex);
}

void media_previous(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	pthread_mutex_lock(&playlist_context->mutex);

	if (uses_song_history_limit(playlist_context) == true) {
		if (playlist_context->queue_history.size() > 0) {
			bool at_start_of_loop_count = false;
			if (playlist_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
				if (playlist_context->infinite == false && playlist_context->loop_count > 0) {
					playlist_context->loop_count--;
				} else {
					at_start_of_loop_count = true;
				}
			} else {
				at_start_of_loop_count = true;
			}
			if (at_start_of_loop_count == true) {
				const MediaData media_data = playlist_context->queue_history[0];

				playlist_context->queue_history.pop_front();

				push_queue_media_data_front(&playlist_context->queue, media_data, playlist_data);

				bool restart = playlist_context->queue[0]->media_data.path ==
					       get_current_media_input(playlist_context->media_source_settings);

				set_queue(playlist_context);

				if (restart) {
					obs_source_media_restart(playlist_context->media_source);
				}
				obs_source_media_started(playlist_context->source);
			}
		}
	} else if (playlist_context->end_behavior == END_BEHAVIOR_LOOP) {
		SharedQueueMediaData new_entry = pop_queue_media_back(&playlist_context->queue);
		if (new_entry != nullptr) {
			if (playlist_context->queue.size() > 0) {
				init_widgets(new_entry, 0, playlist_data);
			}
			playlist_context->queue.push_front(new_entry);

			bool restart = playlist_context->queue[0]->media_data.path ==
				       get_current_media_input(playlist_context->media_source_settings);

			set_queue(playlist_context);

			if (restart) {
				obs_source_media_restart(playlist_context->media_source);
			}

			obs_source_media_started(playlist_context->source);
		}
	} else if (playlist_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
		bool at_start_of_loop_count = false;
		if (playlist_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
			if (playlist_context->infinite == false && playlist_context->loop_count > 0) {
				playlist_context->loop_count--;
			} else {
				at_start_of_loop_count = true;
			}
		} else {
			at_start_of_loop_count = true;
		}
		if (at_start_of_loop_count == true) {
			if (playlist_context->queue.size() < playlist_context->all_media.size()) {

				size_t previous_index = playlist_context->all_media.size() - 1;
				if (playlist_context->queue.size() > 0) {
					SharedQueueMediaData queue_media_data = playlist_context->queue[0];
					if (queue_media_data->media_data.index > 0) {
						previous_index = (queue_media_data->media_data.index - 1);
					}
				}

				const MediaData media_data = playlist_context->all_media[previous_index];

				push_queue_media_data_front(&playlist_context->queue, media_data, playlist_data);
			}
		}
		if (playlist_context->queue.size() > 0) {
			bool restart = playlist_context->queue[0]->media_data.path ==
				       get_current_media_input(playlist_context->media_source_settings);

			set_queue(playlist_context);

			if (restart) {
				obs_source_media_restart(playlist_context->media_source);
			}

			obs_source_media_started(playlist_context->source);
		}
	} else {
		if (playlist_context->queue.size() < playlist_context->all_media.size()) {

			size_t previous_index = playlist_context->all_media.size() - 1;
			if (playlist_context->queue.size() > 0) {
				SharedQueueMediaData queue_media_data = playlist_context->queue[0];
				if (queue_media_data->media_data.index > 0) {
					previous_index = (queue_media_data->media_data.index - 1);
				}
			}

			const MediaData media_data = playlist_context->all_media[previous_index];

			push_queue_media_data_front(&playlist_context->queue, media_data, playlist_data);

			bool restart = playlist_context->queue[0]->media_data.path ==
				       get_current_media_input(playlist_context->media_source_settings);

			set_queue(playlist_context);

			if (restart) {
				obs_source_media_restart(playlist_context->media_source);
			}

			obs_source_media_started(playlist_context->source);
		}
	}

	pthread_mutex_unlock(&playlist_context->mutex);
}

int64_t media_get_duration(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	if (playlist_context->media_source != NULL) {
		return obs_source_media_get_duration(playlist_context->media_source);
	}
	return 0;
}

int64_t media_get_time(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	if (playlist_context->media_source != NULL) {
		return obs_source_media_get_time(playlist_context->media_source);
	}
	return 0;
}

void media_set_time(void *data, int64_t miliseconds)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	if (playlist_context->media_source != NULL) {
		obs_source_media_set_time(playlist_context->media_source, miliseconds);
	}
}

enum obs_media_state media_get_state(void *data)
{
	PlaylistData *playlist_data = static_cast<PlaylistData *>(data);
	PlaylistContext *playlist_context = playlist_data->playlist_context;

	if (playlist_context->all_media.size() > 0 && playlist_context->media_source != nullptr) {
		// return playlist_data->playlist_context->state;
		return obs_source_media_get_state(playlist_context->media_source);
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
void obs_enum_audio_monitoring_devices(obs_enum_audio_device_cb cb, PlaylistSource *playlist_data->playlist_context);
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