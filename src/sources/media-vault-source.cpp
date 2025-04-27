#pragma region Main

#include "../include/sources/media-vault-source.hpp"

#define S_FFMPEG_LOG_CHANGES "log_changes"
#define S_FFMPEG_SPEED "speed"
#define S_FFMPEG_IS_HW_DECODING "is_hw_decoding"
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

void refresh_queue_list(MediaVaultData *media_vault_data)
{
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;
	size_t all_size = media_vault_context->all_media.size();
	size_t queue_size = media_vault_context->queue.size();
	size_t max_size = std::max(all_size, queue_size);

	for (size_t i = 0; i < max_size; i++) {
		if (i < queue_size) {
			pop_queue_media_at(&media_vault_context->queue, queue_size - i - 1); // Remove in reverse order
		}
		if (i < all_size) {
			push_queue_media_data_back(&media_vault_context->queue, media_vault_context->all_media[i],
						   media_vault_data);
		}
	}
}

void media_vault_media_source_ended(void *data, const char *signal, calldata_t *callback)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;
	if (media_vault_context->restarting_media_source == false) {
		if (strcmp(signal, "media_ended") == 0) {
			UNUSED_PARAMETER(callback);
			media_vault_context->state = OBS_MEDIA_STATE_ENDED;

			obs_source_media_next(media_vault_context->source);
		}
	}
}

void set_queue(MediaVaultContext *media_vault_context)
{
	if (!media_vault_context || !media_vault_context->media_source || !media_vault_context->media_source_settings)
		return;

	if (media_vault_context->queue.size() <= 0) {
		obs_source_media_stop(media_vault_context->source);
		// obs_log(LOG_INFO, "WE STOPPED IT YAY!!!");
		return;
	}
	if (media_vault_context->all_media.size() <= 0) {
		return;
	}
	// obs_log(LOG_INFO, "WE TRIED SET THE QUEUE YAY!!!");

	// Get video file path from the array
	MediaContext *media_context = &media_vault_context->queue[0]->media_context;

	if (!media_context)
		return;
	// obs_log(LOG_INFO, "WE SET THE QUEUE YAY!!!");

	obs_data_set_string(media_vault_context->media_source_settings, S_FFMPEG_LOCAL_FILE,
			    media_context->path.c_str());
	obs_source_update(media_vault_context->media_source, media_vault_context->media_source_settings);
}

void clear_any_media_playing(MediaVaultContext *media_vault_context)
{
	if (!media_vault_context->media_source || !media_vault_context->media_source_settings)
		return;

	// Set file path in ffmpeg_source
	obs_data_set_string(media_vault_context->media_source_settings, S_FFMPEG_LOCAL_FILE, "");

	// obs_log(LOG_INFO, " PRINTING: /n%s", obs_data_get_json_pretty(media_vault_context->media_source_settings));

	obs_source_update(media_vault_context->media_source, media_vault_context->media_source_settings);

	obs_source_media_stop(media_vault_context->source);
}

void media_vault_audio_callback(void *data, obs_source_t *source, const struct audio_data *audio_data, bool muted)
{
	UNUSED_PARAMETER(muted);
	UNUSED_PARAMETER(source);

	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	pthread_mutex_lock(&media_vault_context->audio_mutex);

	size_t size = audio_data->frames * sizeof(float);
	for (size_t i = 0; i < media_vault_context->num_channels; i++) {
		deque_push_back(&media_vault_context->audio_data[i], audio_data->data[i], size);
	}
	deque_push_back(&media_vault_context->audio_frames, &audio_data->frames, sizeof(audio_data->frames));
	deque_push_back(&media_vault_context->audio_timestamps, &audio_data->timestamp, sizeof(audio_data->timestamp));

	pthread_mutex_unlock(&media_vault_context->audio_mutex);
}

bool uses_media_history_limit(MediaVaultContext *media_vault_context)
{
	return media_vault_context->shuffle_queue == true;
}
#pragma endregion

#pragma region Property Managment

void show_param_queue(MediaVaultData *media_vault_data)
{
	if (media_vault_data->param_media_vault_widget) {
		QWidget *obs_main_window = (QWidget *)obs_frontend_get_main_window();
		QWidget *parent = obs_main_window;

		for (QObject *child : obs_main_window->children()) {
			QWidget *widget = qobject_cast<QWidget *>(child);
			if (widget && widget->objectName() == "OBSBasicProperties" &&
			    widget->windowTitle() ==
				    QString::fromStdString("Properties for '" +
							   media_vault_data->media_vault_context->name + "'")) {
				parent = widget;
				break;
			}
		}

		if (parent != nullptr) {
			QWidget *old_parent = media_vault_data->param_media_vault_widget->parentWidget();
			if (parent != old_parent) {
				media_vault_data->param_media_vault_widget->setParent(parent, Qt::Window);
				QObject::connect(parent, &QObject::destroyed,
						 media_vault_data->param_media_vault_widget, [media_vault_data]() {
							 media_vault_data->param_media_vault_widget->setParent(nullptr);
							 if (media_vault_data->param_media_vault_widget->isVisible() ==
							     true)
								 media_vault_data->param_media_vault_widget->hide();
						 });
				media_vault_data->param_media_vault_widget->setWindowFlags(Qt::Tool);
			}
		}

		if (media_vault_data->param_media_vault_widget->isVisible() == false)
			media_vault_data->param_media_vault_widget->show();
	}
}

bool show_param_queue_button(obs_properties_t *props, obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	show_param_queue(media_vault_data);
	return true;
}

obs_properties_t *make_media_vault_properties(MediaVaultData *media_vault_data)
{
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;
	obs_properties_t *props = obs_properties_create();

	obs_properties_add_button(props, "show_queue", "Show Queue", show_param_queue_button);

	obs_property_t *stretch_mode = obs_properties_add_list(props, "stretch_mode", "Stretch Mode",
							       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	add_enums_to_property_list(stretch_mode, StretchMode, 1);

	obs_properties_add_editable_list(props, "media_vault", "MediaVault", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");

	obs_properties_add_bool(props, "shuffle_queue", "Shuffle Queue");

	obs_property_t *psb_property = obs_properties_add_list(props, "start_behavior", "MediaVault Start Behavior",
							       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	add_enums_to_property_list(psb_property, StartBehavior, 2);

	obs_property_t *peb_property = obs_properties_add_list(props, "end_behavior", "MediaVault End Behavior",
							       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	add_enums_to_property_list(peb_property, EndBehavior, 2);

	if (media_vault_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
		obs_properties_add_bool(props, "infinite", "Infinite");
		if (media_vault_context->infinite == false) {
			obs_properties_add_int(props, "max_loop_count", "Max Loop Count", 0, INT_MAX, 1);

			obs_property_t *leb_property = obs_properties_add_list(props, "loop_end_behavior",
									       "Loop End Behavior", OBS_COMBO_TYPE_LIST,
									       OBS_COMBO_FORMAT_INT);
			add_enums_to_property_list(leb_property, LoopEndBehavior, 3);
		}
	}

	obs_properties_add_int(props, "media_history_limit", "Media History Limit", 0, INT_MAX, 1);

	obs_properties_add_bool(props, "debug", "Debug");
	return props;
}

/**
 * @brief Updates the media_vault data.
 * @param settings The settings of the media_vault source.
 * @return void
 */
void update_media_vault_data(MediaVaultData *media_vault_data, obs_data_t *settings)
{
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	bool update_properties = false;

	media_vault_context->stretch_mode = (e_StretchMode)obs_data_get_int(settings, "stretch_mode");
	if (media_vault_context->debug == true) {
		obs_log(LOG_INFO, "Stretch Mode: %s", StretchMode[media_vault_context->stretch_mode]);
	}

	pthread_mutex_lock(&media_vault_context->mutex);

	obs_data_array_t *obs_media_vault = obs_data_get_array(settings, "media_vault");

	size_t array_size = obs_data_array_count(obs_media_vault);
	size_t pre_media_array_size = media_vault_context->all_media.size();

	if (array_size <= 0) {
		media_vault_context->all_media.clear();
		if (media_vault_context->queue.size() > 0) {
			clear_queue(&media_vault_context->queue);
			set_queue(media_vault_context);
		}
	} else {
		size_t entry_index = 0;
		MediaList removed_medias{};
		MediaList added_medias{};
		for (size_t i = 0; i < array_size; ++i) {
			obs_data_t *data = obs_data_array_item(obs_media_vault, i);

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

					MediaContext new_entry =
						init_media_data_from_path(entry.path().string(), entry_index);
					if (entry_index < media_vault_context->all_media.size()) {
						const MediaContext media_context = media_vault_context->all_media[i];
						if (media_context.path != new_entry.path) {
							if (pre_media_array_size > array_size) {
								removed_medias.push_back(media_context);
							}
							media_vault_context->all_media[entry_index] = new_entry;
						}
					} else {
						push_media_data_at(&media_vault_context->all_media, new_entry,
								   entry_index);
						added_medias.push_back(new_entry);
					}
					entry_index++;
				}
			} else {
				MediaContext new_entry = init_media_data_from_path(std::string(element), entry_index);
				if (entry_index < media_vault_context->all_media.size()) {
					const MediaContext media_context = media_vault_context->all_media[i];
					if (media_context.path != new_entry.path) {
						if (pre_media_array_size > array_size) {
							removed_medias.push_back(media_context);
						}
						media_vault_context->all_media[entry_index] = new_entry;
					}
				} else {
					push_media_data_at(&media_vault_context->all_media, new_entry, entry_index);
					// obs_log(LOG_INFO, "Media Entry: %s, %d", new_entry.path.c_str(),
					// 	new_entry.index);
					added_medias.push_back(new_entry);
				}
				entry_index++;
			}
			obs_data_release(data);
		}

		for (size_t i = media_vault_context->all_media.size(); i-- > entry_index;) {
			MediaContext new_entry = media_vault_context->all_media[i];
			if (pre_media_array_size > array_size) {
				if (removed_medias.size() == 0) {
					removed_medias.push_front(new_entry);
				}
			}
			pop_media_at(&media_vault_context->all_media, i);
		}

		if (media_vault_context->all_media_initialized == true) {
			bool changed_queue = false;
			for (size_t i = media_vault_context->queue.size(); i-- > 0;) {
				QueueMedia queue_media_data = media_vault_context->queue[i];
				if (queue_media_data->media_context.index < media_vault_context->all_media.size()) {
					MediaContext media_context =
						media_vault_context->all_media[queue_media_data->media_context.index];

					if (queue_media_data->media_context.path != media_context.path) {
						QueueMedia new_entry = init_queue_media_data_from_media_data(
							media_context, i, media_vault_data,
							queue_media_data->media_widget,
							queue_media_data->param_media_widget);

						media_vault_context->queue[i] = new_entry;

						new_entry->media_widget->update_media_data();
						new_entry->param_media_widget->update_media_data();

						if (changed_queue == false)
							changed_queue = true;
					}
				}
			}
			for (size_t i = media_vault_context->queue.size(); i-- > 0;) {
				QueueMedia queue_media_data = media_vault_context->queue[i];
				for (size_t r_i = removed_medias.size(); r_i-- > 0;) {
					MediaContext removed_media_data = removed_medias[r_i];
					if (queue_media_data->media_context.index == removed_media_data.index) {
						pop_queue_media_at(&media_vault_context->queue, i, true);
						if (changed_queue == false)
							changed_queue = true;
					}
				}
			}
			bool found_queue = false;
			size_t queue_last_index = 0;

			if (media_vault_context->queue.size() > 0) {
				queue_last_index = media_vault_context->queue[media_vault_context->queue.size() - 1]
							   ->media_context.index;
				found_queue = true;
			}

			// to-do Only add songs that have actually been added to the all media list.
			for (size_t i = 0; i < added_medias.size(); i++) {
				// obs_log(LOG_INFO, "Queue Index: %d", i);
				MediaContext added_media_data = added_medias[i];
				if (added_media_data.index > queue_last_index || found_queue == false) {
					push_queue_media_data_back(&media_vault_context->queue, added_media_data,
								   media_vault_data);
					if (changed_queue == false)
						changed_queue = true;
				}
			}
			if (changed_queue == true) {
				set_queue(media_vault_context);
				if (found_queue == false) {
					media_vault_context->restarting_media_source = true;
					obs_source_media_restart(media_vault_context->media_source);
					media_vault_context->restarting_media_source = false;
				}
			}
		}
	}

	obs_data_array_release(obs_media_vault);

	if (media_vault_context->all_media_initialized == false) {
		media_vault_context->all_media_initialized = true;
	}

	pthread_mutex_unlock(&media_vault_context->mutex);

	if (media_vault_context->debug == true) {
		obs_log_media_array(LOG_INFO, "All Media:/n", &media_vault_context->all_media, 0, "    ",
				    MEDIA_STRINGIFY_TYPE_FILENAME);
	}

	bool is_queue_shuffle = obs_data_get_bool(settings, "shuffle_queue");
	bool shuffle_changed = is_queue_shuffle != media_vault_context->shuffle_queue;

	if (shuffle_changed)
		media_vault_context->shuffle_queue = is_queue_shuffle;

	media_vault_context->start_behavior = (e_StartBehavior)obs_data_get_int(settings, "start_behavior");
	if (media_vault_context->debug == true) {
		obs_log(LOG_INFO, "Start Behavior: %s", StartBehavior[media_vault_context->start_behavior]);
	}

	e_EndBehavior end_behavior = (e_EndBehavior)obs_data_get_int(settings, "end_behavior");

	bool end_behavior_changed = media_vault_context->end_behavior != end_behavior;

	if (end_behavior_changed) {
		update_properties = true;
	}

	media_vault_context->end_behavior = end_behavior;
	if (media_vault_context->debug == true) {
		obs_log(LOG_INFO, "End Behavior: %s", EndBehavior[media_vault_context->end_behavior]);
	}

	int all_media_size = (int)media_vault_context->all_media.size();

	int last_index = all_media_size - 1;

	if (last_index < 0) {
		last_index += 1;
	}

	bool infinite_changed = false;
	bool infinite = obs_data_get_bool(settings, "infinite");
	if (media_vault_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
		if (infinite != media_vault_context->infinite) {
			infinite_changed = true;
			update_properties = true;
		}
		if (infinite == false) {
			media_vault_context->max_loop_count = (int)obs_data_get_int(settings, "max_loop_count");
			media_vault_context->loop_end_behavior =
				(e_LoopEndBehavior)obs_data_get_int(settings, "loop_end_behavior");
		}
		if (media_vault_data->media_vault_context->debug == true) {
			obs_log(LOG_INFO, "Infinite: %s", infinite == true ? "true" : "false");
			if (infinite == false && media_vault_context->debug == true) {
				obs_log(LOG_INFO, "Max Loop Count: %d", media_vault_context->max_loop_count);
			}
		}
	}
	media_vault_context->infinite = infinite;

	pthread_mutex_lock(&media_vault_context->mutex);

	switch (media_vault_context->end_behavior) {
	case END_BEHAVIOR_STOP:
		if (is_queue_shuffle == false) {
			// if (end_behavior_changed == true) {
			// 	obs_source_media_restart(media_vault_context->source);
			// }
			// } else {
			if (end_behavior_changed == true || shuffle_changed == true) {
				obs_source_media_restart(media_vault_context->source);
			} else if (media_vault_context->queue.size() > 0) {
				QueueMedia first_queue_media = media_vault_context->queue[0];
				for (size_t i = media_vault_context->queue.size(); i-- > 0;) {
					QueueMedia queue_media_data = media_vault_context->queue[i];
					if (queue_media_data->media_context.index <
					    first_queue_media->media_context.index) {
						pop_queue_media_at(&media_vault_context->queue, i);
					}
				}
			}
		} else {
			if (end_behavior_changed == true || shuffle_changed == true) {
				obs_source_media_restart(media_vault_context->source);
			}
		}
		break;
	case END_BEHAVIOR_LOOP:
		if (is_queue_shuffle == false) {
			// } else {
			if (shuffle_changed == true) {
				obs_source_media_restart(media_vault_context->source);
			} else if (media_vault_context->queue.size() < media_vault_context->all_media.size()) {
				if (media_vault_context->queue.size() > 0) {
					QueueMedia first_queue_media_data = media_vault_context->queue[0];

					size_t queue_index = 1;

					for (size_t i = first_queue_media_data->media_context.index + 1;
					     i < media_vault_context->all_media.size(); i++) {
						MediaContext media_context = media_vault_context->all_media[i];
						if (queue_index < media_vault_context->queue.size()) {
							QueueMedia queue_media_data =
								media_vault_context->queue[queue_index];

							if (queue_media_data->media_context.path !=
							    media_context.path) {
								QueueMedia new_entry =
									init_queue_media_data_from_media_data(
										media_context, i, media_vault_data,
										queue_media_data->media_widget,
										queue_media_data->param_media_widget);

								media_vault_context->queue[queue_index] = new_entry;

								new_entry->media_widget->update_media_data();
								new_entry->param_media_widget->update_media_data();
							}
						} else {
							push_queue_media_data_at(&media_vault_context->queue,
										 media_context, queue_index,
										 media_vault_data);
						}
						queue_index++;
					}
					for (size_t i = 0; i < first_queue_media_data->media_context.index; i++) {
						MediaContext media_context = media_vault_context->all_media[i];
						if (queue_index < media_vault_context->queue.size()) {
							QueueMedia queue_media_data =
								media_vault_context->queue[queue_index];

							if (queue_media_data->media_context.path !=
							    media_context.path) {
								QueueMedia new_entry =
									init_queue_media_data_from_media_data(
										media_context, i, media_vault_data,
										queue_media_data->media_widget,
										queue_media_data->param_media_widget);

								media_vault_context->queue[queue_index] = new_entry;

								new_entry->media_widget->update_media_data();
								new_entry->param_media_widget->update_media_data();
							}
						} else {
							push_queue_media_data_at(&media_vault_context->queue,
										 media_context, queue_index,
										 media_vault_data);
						}
						queue_index++;
					}
				} else {
					obs_source_media_restart(media_vault_context->source);
				}
			}
		}
		break;
	case END_BEHAVIOR_LOOP_AT_END:
		if (infinite_changed == true) {
			media_vault_context->loop_count = 0;
			if (media_vault_context->infinite) {
				if (media_vault_context->queue.size() <= 0) {
					obs_source_media_previous(media_vault_context->source);
				}
			}
		}
		if (is_queue_shuffle == false) {
			if (end_behavior_changed == true || shuffle_changed == true) {
				obs_log(LOG_INFO, "This should restart");
				obs_source_media_restart(media_vault_context->source);
			} else if (media_vault_context->queue.size() > 0) {
				QueueMedia first_queue_media = media_vault_context->queue[0];
				for (size_t i = media_vault_context->queue.size(); i-- > 0;) {
					QueueMedia queue_media_data = media_vault_context->queue[i];
					if (queue_media_data->media_context.index <
					    first_queue_media->media_context.index) {
						pop_queue_media_at(&media_vault_context->queue, i);
					}
				}
			}
		} else {
			if (end_behavior_changed == true || shuffle_changed == true) {
				obs_log(LOG_INFO, "This should restart");
				obs_source_media_restart(media_vault_context->source);
			}
		}
		break;
	default:
		break;
	}

	pthread_mutex_unlock(&media_vault_context->mutex);

	media_vault_context->media_history_limit = (int)obs_data_get_int(settings, "media_history_limit");

	media_vault_context->debug = obs_data_get_bool(settings, "debug");
	if (media_vault_context->debug == true) {
		obs_log(LOG_INFO, "Debug: %s", media_vault_context->debug ? "true" : "false");
	}

	if (update_properties == true) {
		obs_source_update_properties(media_vault_context->source);
	}
}

#pragma endregion

#pragma region MediaVault Main Functions

const char *media_vault_source_name(void *data)
{
	return "MediaVault"; // This should match the filename (without extension) in data/
}

void *media_vault_source_create(obs_data_t *settings, obs_source_t *source)
{
	MediaVaultData *media_vault_data = new MediaVaultData();
	MediaVaultContext *media_vault_context = new MediaVaultContext();
	media_vault_data->media_vault_context = media_vault_context;

	std::stringstream ss;
	ss << obs_source_get_name(source);
	media_vault_context->name = ss.str();

	media_vault_context->source = source;

	media_vault_context->media_source_settings = obs_data_create();

	obs_data_set_bool(media_vault_context->media_source_settings, S_FFMPEG_LOG_CHANGES, false);
	obs_data_set_bool(media_vault_context->media_source_settings, S_FFMPEG_RESTART_PLAYBACK_ON_ACTIVE, false);
	media_vault_context->media_source =
		obs_source_create_private("ffmpeg_source", "Video Source", media_vault_context->media_source_settings);

	obs_source_add_active_child(media_vault_context->source, media_vault_context->media_source);
	obs_source_add_audio_capture_callback(media_vault_context->media_source, media_vault_audio_callback,
					      media_vault_data);

	// signal_handler_t *sh_source = obs_source_get_signal_handler(media_vault_context->source);
	// signal_handler_connect_global(sh_source, media_vault_source_callbacks, media_vault_context);

	signal_handler_t *sh_media_source = obs_source_get_signal_handler(media_vault_context->media_source);
	// signal_handler_connect(sh_media_source, "media_ended", media_vault_media_source_ended, media_vault_data);
	signal_handler_connect_global(sh_media_source, media_vault_media_source_ended, media_vault_data);

	media_vault_context->all_media_initialized = false;

	media_vault_context->all_media = {};

	media_vault_context->queue = {};

	media_vault_context->queue_history = {};

	// media_vault_context->current_media = NULL;
	// media_vault_context->current_media_index = 0;
	media_vault_context->state = OBS_MEDIA_STATE_NONE;
	media_vault_context->num_channels = 2;
	media_vault_context->loop_index = 0;
	media_vault_context->infinite = true;
	media_vault_context->max_loop_count = -1;
	media_vault_context->loop_count = 0;
	media_vault_context->restarting_media_source = false;

	// media_vault_context->properties = make_media_vault_properties(media_vault_context);

	// QWidget *properties_window = (QWidget *)obs_frontend_get_main_window();
	// media_vault_context->properties_ui = new CustomProperties(settings, properties_window);

	// pthread_mutex_init_value(&media_vault_context->mutex);
	// QWidget *obs_main_window = (QWidget *)obs_frontend_get_main_window();
	if (pthread_mutex_init(&media_vault_context->mutex, NULL) != 0) {
		goto error;
	}

	// pthread_mutex_init_value(&media_vault_context->audio_mutex);
	if (pthread_mutex_init(&media_vault_context->audio_mutex, NULL) != 0) {
		goto error;
	}

	update_media_vault_data(media_vault_data, settings);

	media_vault_data->param_media_vault_widget = new MediaVaultQueueWidget(media_vault_context, nullptr, true);
	media_vault_data->param_media_vault_widget->setObjectName("media_vault_queue_viewer_maximon9");

	media_vault_data->media_vault_widget =
		new MediaVaultQueueWidget(media_vault_context, multi_media_vault_queue_viewer, false);
	multi_media_vault_queue_viewer->addMediaVaultWidget(media_vault_data->media_vault_widget);
	multi_media_vault_queue_viewer->media_vault_datas.push_back(media_vault_data);

	return media_vault_data;
error:
	media_vault_source_destroy(media_vault_data);
	return NULL;
}

void media_vault_source_destroy(void *data)
{
	// obs_log(LOG_INFO, "Destroying MediaVault");
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	if (media_vault_context->media_source_settings != NULL) {
		obs_data_release(media_vault_context->media_source_settings);
	}
	if (media_vault_context->media_source != NULL) {
		obs_source_release(media_vault_context->media_source);
	}

	for (size_t i = 0; i < MAX_AUDIO_CHANNELS; i++) {
		deque_free(&media_vault_context->audio_data[i]);
	}

	deque_free(&media_vault_context->audio_frames);
	deque_free(&media_vault_context->audio_timestamps);

	pthread_mutex_destroy(&media_vault_context->mutex);
	pthread_mutex_destroy(&media_vault_context->audio_mutex);

	if (media_vault_data->media_vault_widget != nullptr) {
		media_vault_data->media_vault_widget->remove_widget();
		// delete media_vault_data->media_vault_widget;
	}
	if (media_vault_data->param_media_vault_widget != nullptr) {
		media_vault_data->param_media_vault_widget->remove_widget();
		// delete media_vault_data->param_media_vault_widget;
	}

	delete media_vault_context;
	delete media_vault_data;
	obs_source_release(media_vault_context->source);
}

uint32_t media_vault_source_width(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;
	return obs_source_get_width(media_vault_context->media_source);
}

uint32_t media_vault_source_height(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;
	return obs_source_get_height(media_vault_context->media_source);
}

void media_vault_get_defaults(obs_data_t *settings)
{
	// obs_data_set_default_int(settings, "start_behavior", 0);
	// obs_data_set_default_int(settings, "end_behavior", 0);
	// obs_data_set_default_int(settings, "loop_index", 0);
	obs_data_set_default_bool(settings, "show_queue_when_properties_open", true);
	obs_data_set_default_bool(settings, "shuffle_queue", false);
	obs_data_set_default_bool(settings, "infinite", true);
	obs_data_set_default_int(settings, "max_loop_count", 0);
	obs_data_set_default_bool(settings, "debug", false);
	obs_data_set_default_int(settings, "media_history_limit", 100);
}

obs_properties_t *media_vault_get_properties(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	return make_media_vault_properties(media_vault_data);
}

void media_vault_update(void *data, obs_data_t *settings)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);

	update_media_vault_data(media_vault_data, settings);
}

void media_vault_activate(void *data)
{
	obs_log(LOG_INFO, "media_vault_activate");
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	obs_source_t *scene_source = obs_frontend_get_current_scene();

	media_vault_context->source_scene_item =
		obs_scene_find_source_recursive(obs_scene_from_source(scene_source), media_vault_context->name.c_str());

	obs_source_release(scene_source);

	switch (media_vault_context->start_behavior) {
	case START_BEHAVIOR_RESTART_ENTIRE_PLAYLIST:
		obs_source_media_restart(media_vault_context->source);
		obs_source_media_started(media_vault_context->source);
		break;
	case START_BEHAVIOR_RESTART_AT_CURRENT_INDEX:
		set_queue(media_vault_context);
		media_vault_context->restarting_media_source = true;
		obs_source_media_restart(media_vault_context->media_source);
		media_vault_context->restarting_media_source = false;
		obs_source_media_started(media_vault_context->source);
		break;
	case START_BEHAVIOR_UNPAUSE:
		set_queue(media_vault_context);
		obs_source_media_play_pause(media_vault_context->source, false);
		break;
	case START_BEHAVIOR_PAUSE:
		obs_source_media_play_pause(media_vault_context->source, true);
		break;
	default:
		break;
	}
	// obs_source_update_properties(media_vault_data->media_vault_context->source);
}

void media_vault_deactivate(void *data)
{
	obs_log(LOG_INFO, "media_vault_deactivate");
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	switch (media_vault_context->start_behavior) {
	case START_BEHAVIOR_RESTART_ENTIRE_PLAYLIST:
		obs_source_media_stop(media_vault_context->source);
		obs_source_media_ended(media_vault_context->source);
		break;
	case START_BEHAVIOR_RESTART_AT_CURRENT_INDEX:
		obs_source_media_stop(media_vault_context->media_source);
		obs_source_media_ended(media_vault_context->source);
		break;
	case START_BEHAVIOR_UNPAUSE:
		obs_source_media_play_pause(media_vault_context->source, true);
		break;
	case START_BEHAVIOR_PAUSE:
		obs_source_media_play_pause(media_vault_context->source, true);
		break;
	default:
		break;
	}
}

void media_vault_video_tick(void *data, float seconds)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	//UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(seconds);

	const audio_t *a = obs_get_audio();
	const struct audio_output_info *aoi = audio_output_get_info(a);

	pthread_mutex_lock(&media_vault_context->audio_mutex);

	while (media_vault_context->audio_frames.size > 0) {
		struct obs_source_audio audio = {0};
		audio.format = aoi->format;
		audio.samples_per_sec = aoi->samples_per_sec;
		audio.speakers = aoi->speakers;

		deque_pop_front(&media_vault_context->audio_frames, &audio.frames, sizeof(audio.frames));
		deque_pop_front(&media_vault_context->audio_timestamps, &audio.timestamp, sizeof(audio.timestamp));

		for (size_t i = 0; i < media_vault_context->num_channels; i++) {
			audio.data[i] = (uint8_t *)media_vault_context->audio_data[i].data +
					media_vault_context->audio_data[i].start_pos;
		}
		obs_source_output_audio(media_vault_context->source, &audio);
		for (size_t i = 0; i < media_vault_context->num_channels; i++) {
			deque_pop_front(&media_vault_context->audio_data[i], NULL, audio.frames * sizeof(float));
		}
	}

	media_vault_context->num_channels = audio_output_get_channels(a);

	pthread_mutex_unlock(&media_vault_context->audio_mutex);

	handle_stretch_mode(media_vault_context);
}

void media_vault_video_render(void *data, gs_effect_t *effect)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	if (media_vault_context->media_source != NULL) {
		obs_source_video_render(media_vault_context->media_source);
	} else {
		obs_source_video_render(NULL);
	}

	UNUSED_PARAMETER(effect);
	// obs_log(LOG_INFO, "video_render");
}

bool media_vault_audio_render(void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio_output, uint32_t mixers,
			      size_t channels, size_t sample_rate)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	if (!media_vault_context->media_source)
		return false;

	uint64_t source_ts = obs_source_get_audio_timestamp(media_vault_context->media_source);
	if (!source_ts)
		return false;

	// Get the audio from the child source (ffmpeg_source)
	struct obs_source_audio_mix child_audio_output = {0};

	// Retrieve the audio mix from the child source
	obs_source_get_audio_mix(media_vault_context->media_source, &child_audio_output);

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

/* obs_properties_t *media_vault_get_properties2(void *data, void *type_data)
{
	obs_log(LOG_INFO, "Yo wassup");

	return nullptr;
} */

void media_vault_enum_active_sources(void *data, obs_source_enum_proc_t enum_callback, void *param)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	pthread_mutex_lock(&media_vault_context->mutex);
	enum_callback(media_vault_context->source, media_vault_context->media_source, param);
	pthread_mutex_unlock(&media_vault_context->mutex);
}

void media_vault_save(void *data, obs_data_t *settings)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	obs_log(LOG_INFO, "media_vault_save");

	media_vault_context->name = obs_source_get_name(media_vault_context->source);
	if (media_vault_data->media_vault_widget != nullptr) {
		media_vault_data->media_vault_widget->update_media_vault_name();
	}
	if (media_vault_data->param_media_vault_widget != nullptr) {
		media_vault_data->param_media_vault_widget->update_media_vault_name();
	}

	// 	if (media_vault_data->param_media_vault_widget) {
	// 		media_vault_data->param_media_vault_widget->hide();
	// 	}
}

void media_vault_load(void *data, obs_data_t *settings)
{
	obs_log(LOG_INFO, "media_vault_load");
}

void media_play_pause(void *data, bool pause)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	// pthread_mutex_lock(&media_vault_context->mutex);

	if (media_vault_context->media_source != NULL) {
		if (pause == true) {
			media_vault_context->state = OBS_MEDIA_STATE_PAUSED;
		} else {
			media_vault_context->state = OBS_MEDIA_STATE_PLAYING;
		}
		obs_source_media_play_pause(media_vault_context->media_source, pause);
	}

	// pthread_mutex_unlock(&media_vault_context->mutex);
}

void media_restart(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	// pthread_mutex_lock(&media_vault_context->mutex);

	if (media_vault_context->media_source != NULL) {
		media_vault_context->loop_count = 0;

		refresh_queue_list(media_vault_data);

		if (uses_media_history_limit(media_vault_context)) {
			shuffle_queue(&media_vault_context->queue, media_vault_data);
			media_vault_context->queue_history.clear();
		}

		set_queue(media_vault_context);

		if (media_vault_context->queue.size() > 0) {
			media_vault_context->state = OBS_MEDIA_STATE_PLAYING;
			media_vault_context->restarting_media_source = true;
			obs_source_media_restart(media_vault_context->media_source);
			media_vault_context->restarting_media_source = false;
		}
	}

	// pthread_mutex_unlock(&media_vault_context->mutex);
}

void media_stop(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	// pthread_mutex_lock(&media_vault_context->mutex);

	if (media_vault_context->media_source != NULL) {
		media_vault_context->state = OBS_MEDIA_STATE_STOPPED;

		if (uses_media_history_limit(media_vault_context)) {
			for (size_t i = 0; i < media_vault_context->queue.size(); i++) {
				const MediaContext media_context = media_vault_context->queue[i]->media_context;
				push_media_data_front(&media_vault_context->queue_history, media_context);
				if (media_vault_context->queue_history.size() >
				    media_vault_context->media_history_limit) {
					media_vault_context->queue_history.pop_back();
				}
			}
		}

		clear_queue(&media_vault_context->queue);
		obs_source_media_stop(media_vault_context->media_source);
	}

	// pthread_mutex_unlock(&media_vault_context->mutex);
}

void media_next(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	pthread_mutex_lock(&media_vault_context->mutex);

	// obs_log(LOG_INFO, "Is Infinite: %s", media_vault_context->infinite == true ? "true" : "else");
	// obs_log(LOG_INFO, "End Behavior: %s", EndBehavior[media_vault_context->end_behavior]);
	// obs_log(LOG_INFO, "Shuffle Queue: %s", media_vault_context->shuffle_queue == true ? "true" : "false");
	if (media_vault_context->queue.size() > 0) {
		if (uses_media_history_limit(media_vault_context) == true) {
			if (media_vault_context->end_behavior != END_BEHAVIOR_LOOP_AT_END ||
			    (media_vault_context->end_behavior == END_BEHAVIOR_LOOP_AT_END &&
			     media_vault_context->queue.size() > 1)) {
				const MediaContext media_context = media_vault_context->queue[0]->media_context;

				push_media_data_front(&media_vault_context->queue_history, media_context);

				if (media_vault_context->queue_history.size() >
				    media_vault_context->media_history_limit) {
					media_vault_context->queue_history.pop_back();
				}
			}

			switch (media_vault_context->end_behavior) {
			case END_BEHAVIOR_LOOP: {
				pop_queue_media_front(&media_vault_context->queue);
				if (media_vault_context->all_media.size() > 0) {
					if (media_vault_context->queue.size() < media_vault_context->all_media.size()) {
						MediaContext random_media_data =
							media_vault_context->all_media[get_random_size_t(
								0, media_vault_context->all_media.size() - 1)];
						push_queue_media_data_back(&media_vault_context->queue,
									   random_media_data, media_vault_data);
					}
				}
				break;
			}
			case END_BEHAVIOR_LOOP_AT_END: {
				if (media_vault_context->infinite == false && media_vault_context->queue.size() == 1) {
					media_vault_context->loop_count++;
				}
				size_t queue_size = media_vault_context->queue.size();
				if (queue_size > 1) {
					pop_queue_media_front(&media_vault_context->queue);
				}
				if (media_vault_context->infinite == false) {
					if (queue_size <= 1 &&
					    media_vault_context->loop_count >= media_vault_context->max_loop_count) {

						const MediaContext media_context =
							media_vault_context->queue[0]->media_context;

						push_media_data_front(&media_vault_context->queue_history,
								      media_context);

						pop_queue_media_front(&media_vault_context->queue);
					}
				}
				break;
			}
			default: {
				pop_queue_media_front(&media_vault_context->queue);
				break;
			}
			}
		} else if (media_vault_context->end_behavior == END_BEHAVIOR_LOOP) {
			if (media_vault_context->queue.size() > 1) {
				QueueMedia new_entry = pop_queue_media_front(&media_vault_context->queue);
				if (media_vault_context->queue.size() > 0) {
					init_widgets(new_entry, media_vault_context->queue.size(), media_vault_data);
				}
				media_vault_context->queue.push_back(new_entry);
			}
		} else if (media_vault_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
			if (media_vault_context->infinite == false && media_vault_context->queue.size() == 1) {
				media_vault_context->loop_count++;
			}
			size_t queue_size = media_vault_context->queue.size();
			if (queue_size > 1) {
				pop_queue_media_front(&media_vault_context->queue);
			}
			if (media_vault_context->infinite == false) {
				if (queue_size <= 1 &&
				    media_vault_context->loop_count >= media_vault_context->max_loop_count) {
					pop_queue_media_front(&media_vault_context->queue);
				}
			}
		} else {
			pop_queue_media_front(&media_vault_context->queue);
		}

		if (media_vault_context->queue.size() > 0) {
			bool set_this_queue = false;
			if (media_vault_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
				if (media_vault_context->infinite == false) {
					if (media_vault_context->loop_count < media_vault_context->max_loop_count ||
					    media_vault_context->max_loop_count == 0) {
						set_this_queue = true;
					}
				} else {
					set_this_queue = true;
				}
			} else {
				set_this_queue = true;
			}
			if (set_this_queue == true) {
				bool restart = media_vault_context->queue[0]->media_context.path ==
					       get_current_media_input(media_vault_context->media_source_settings);

				set_queue(media_vault_context);

				if (restart) {
					media_vault_context->restarting_media_source = true;
					obs_source_media_restart(media_vault_context->media_source);
					media_vault_context->restarting_media_source = false;
				}
			}
		} else {
			obs_source_media_stop(media_vault_context->media_source);
			obs_source_media_ended(media_vault_context->source);
		}
	} else {
		obs_source_media_stop(media_vault_context->media_source);
		obs_source_media_ended(media_vault_context->source);
	}
	bool media_ended = media_vault_context->state == OBS_MEDIA_STATE_STOPPED ||
			   media_vault_context->state == OBS_MEDIA_STATE_ENDED;
	if (!media_ended) {
		obs_source_media_play_pause(media_vault_context->source, false);
	}

	pthread_mutex_unlock(&media_vault_context->mutex);
}

void media_previous(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	pthread_mutex_lock(&media_vault_context->mutex);

	bool restart = false;
	bool un_pause = true;

	if (uses_media_history_limit(media_vault_context) == true) {
		bool history_size_not_empty = media_vault_context->queue_history.size() > 0;
		obs_log(LOG_INFO, "HISTORY TEST %d, %s", media_vault_context->queue_history.size(),
			history_size_not_empty ? "true" : "false");
		if (history_size_not_empty) {
			bool at_start_of_loop_count = false;
			if (media_vault_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
				if (media_vault_context->infinite == false && media_vault_context->loop_count > 0) {
					media_vault_context->loop_count--;
					if (media_vault_context->queue.size() <= 0) {
						const MediaContext media_context =
							media_vault_context->queue_history[0];

						media_vault_context->queue_history.pop_front();

						push_queue_media_data_front(&media_vault_context->queue, media_context,
									    media_vault_data);
					}
				} else {
					at_start_of_loop_count = true;
				}
			} else {
				at_start_of_loop_count = true;
			}
			if (at_start_of_loop_count == true) {
				const MediaContext media_context = media_vault_context->queue_history[0];

				media_vault_context->queue_history.pop_front();

				push_queue_media_data_front(&media_vault_context->queue, media_context,
							    media_vault_data);
			}
		} else {
			un_pause = false;
		}

		if (media_vault_context->queue.size() > 0) {
			restart = media_vault_context->queue[0]->media_context.path ==
				  get_current_media_input(media_vault_context->media_source_settings);
		}
		if (history_size_not_empty) {
			if (media_vault_context->queue.size() > 0) {

				set_queue(media_vault_context);

				obs_source_media_started(media_vault_context->source);
			}
		}
	} else if (media_vault_context->end_behavior == END_BEHAVIOR_LOOP) {
		QueueMedia new_entry = pop_queue_media_back(&media_vault_context->queue);
		if (new_entry != nullptr) {
			if (media_vault_context->queue.size() > 0) {
				init_widgets(new_entry, 0, media_vault_data);
			}
			media_vault_context->queue.push_front(new_entry);

			restart = media_vault_context->queue[0]->media_context.path ==
				  get_current_media_input(media_vault_context->media_source_settings);

			set_queue(media_vault_context);

			obs_source_media_started(media_vault_context->source);
		}
	} else if (media_vault_context->end_behavior == END_BEHAVIOR_LOOP_AT_END) {
		bool at_start_of_loop_count = false;
		if (media_vault_context->infinite == false && media_vault_context->loop_count > 0) {
			media_vault_context->loop_count--;
			if (media_vault_context->queue.size() <= 0) {
				size_t previous_index = media_vault_context->all_media.size() - 1;

				const MediaContext media_context = media_vault_context->all_media[previous_index];

				push_queue_media_data_front(&media_vault_context->queue, media_context,
							    media_vault_data);
			}
		} else {
			at_start_of_loop_count = true;
		}

		bool queue_size_under_all_media_size = media_vault_context->queue.size() <
						       media_vault_context->all_media.size();
		if (at_start_of_loop_count == true) {
			if (queue_size_under_all_media_size) {

				size_t previous_index = media_vault_context->all_media.size() - 1;
				if (media_vault_context->queue.size() > 0) {
					QueueMedia queue_media_data = media_vault_context->queue[0];
					if (queue_media_data->media_context.index > 0) {
						previous_index = (queue_media_data->media_context.index - 1);
					}
				}

				const MediaContext media_context = media_vault_context->all_media[previous_index];

				push_queue_media_data_front(&media_vault_context->queue, media_context,
							    media_vault_data);
			}
		}

		if (media_vault_context->queue.size() > 0) {
			restart = media_vault_context->queue[0]->media_context.path ==
				  get_current_media_input(media_vault_context->media_source_settings);
		}

		if (queue_size_under_all_media_size) {
			set_queue(media_vault_context);

			obs_source_media_started(media_vault_context->source);
		}

	} else {
		bool queue_size_under_all_media_size = media_vault_context->queue.size() <
						       media_vault_context->all_media.size();
		if (queue_size_under_all_media_size) {

			size_t previous_index = media_vault_context->all_media.size() - 1;
			if (media_vault_context->queue.size() > 0) {
				QueueMedia queue_media_data = media_vault_context->queue[0];
				if (queue_media_data->media_context.index > 0) {
					previous_index = (queue_media_data->media_context.index - 1);
				}
			}

			const MediaContext media_context = media_vault_context->all_media[previous_index];

			push_queue_media_data_front(&media_vault_context->queue, media_context, media_vault_data);
		}

		if (media_vault_context->queue.size() > 0) {
			restart = media_vault_context->queue[0]->media_context.path ==
				  get_current_media_input(media_vault_context->media_source_settings);
		}

		if (queue_size_under_all_media_size) {
			set_queue(media_vault_context);

			obs_source_media_started(media_vault_context->source);
		}
	}

	if (restart) {
		media_vault_context->restarting_media_source = true;
		obs_source_media_restart(media_vault_context->media_source);
		media_vault_context->restarting_media_source = false;
	}

	bool media_ended = media_vault_context->state == OBS_MEDIA_STATE_STOPPED ||
			   media_vault_context->state == OBS_MEDIA_STATE_ENDED;

	if (((media_vault_context->end_behavior == END_BEHAVIOR_LOOP && !media_ended) ||
	     media_vault_context->end_behavior != END_BEHAVIOR_LOOP) &&
	    un_pause) {
		obs_source_media_play_pause(media_vault_context->source, false);
	}

	pthread_mutex_unlock(&media_vault_context->mutex);
}

int64_t media_get_duration(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	if (media_vault_context->media_source != NULL) {
		return obs_source_media_get_duration(media_vault_context->media_source);
	}
	return 0;
}

int64_t media_get_time(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	if (media_vault_context->media_source != NULL) {
		return obs_source_media_get_time(media_vault_context->media_source);
	}
	return 0;
}

void media_set_time(void *data, int64_t miliseconds)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	if (media_vault_context->media_source != NULL) {
		obs_source_media_set_time(media_vault_context->media_source, miliseconds);
	}
}

enum obs_media_state media_get_state(void *data)
{
	MediaVaultData *media_vault_data = static_cast<MediaVaultData *>(data);
	MediaVaultContext *media_vault_context = media_vault_data->media_vault_context;

	if (media_vault_context->all_media.size() > 0 && media_vault_context->media_source != nullptr) {
		// return media_vault_data->media_vault_context->state;
		return obs_source_media_get_state(media_vault_context->media_source);
	}
	return OBS_MEDIA_STATE_NONE;
}

#pragma endregion

#pragma endregion

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