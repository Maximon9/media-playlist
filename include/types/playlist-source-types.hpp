#ifndef PLAYLIST_SOURCE_TYPES_HPP
#define PLAYLIST_SOURCE_TYPES_HPP

#include <obs-module.h>
#include <plugin-support.h>
#include <util/threading.h>
#include <util/deque.h>
#include "./media-file-data-types.hpp"
#include "../include/utils/enum-utils.hpp"
#include "../include/qt-classes/playlist-queue-widget.hpp"

class PlaylisQueueWidget;

typedef struct PlaylistData {
#pragma region Public
	bool show_queue_when_properties_open;
	MediaDataArray all_media;
	obs_source_t *source;
	obs_source_t *media_source;
	bool shuffle_queue;
	e_StartBehavior start_behavior;
	e_EndBehavior end_behavior;
	int loop_index;
	bool infinite;
	int loop_count;
	e_LoopEndBehavior loop_end_behavior;
	int song_history_limit;
	bool debug;
#pragma endregion
#pragma region Private
	obs_media_state state;
	std::string name;
	bool all_media_initialized;
	obs_data_t *media_source_settings;
	QueueMediaDataArray queue;
	MediaDataArray previous_queue;
	pthread_mutex_t mutex;
	struct deque audio_data[MAX_AUDIO_CHANNELS];
	struct deque audio_frames;
	struct deque audio_timestamps;
	size_t num_channels;
	pthread_mutex_t audio_mutex;
#pragma endregion
} PlaylistData;

typedef struct PlaylistWidgetData {
	QWidget *test_widget;
	PlaylisQueueWidget *playlist_widget;
	PlaylisQueueWidget *param_playlist_widget;
	PlaylistData *playlist_data;
} PlaylistWidgetData;

#endif // PLAYLIST_SOURCE_TYPES_HPP