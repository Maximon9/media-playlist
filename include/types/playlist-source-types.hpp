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

typedef struct PlaylistContext {
#pragma region Public
	e_StretchMode stretch_mode;
	MediaDataArray all_media;
	obs_source_t *source;
	obs_source_t *media_source;
	bool shuffle_queue;
	e_StartBehavior start_behavior;
	e_EndBehavior end_behavior;
	int loop_index;
	bool infinite;
	int max_loop_count;
	e_LoopEndBehavior loop_end_behavior;
	int song_history_limit;
	bool debug;
#pragma endregion
#pragma region Private
	obs_sceneitem_t *source_scene_item;
	bool use_media_resolution;
	int media_width;
	int media_height;
	bool restarting_media_source;
	int loop_count;
	obs_media_state state;
	std::string name;
	bool all_media_initialized;
	obs_data_t *media_source_settings;
	QueueMediaDataArray queue;
	MediaDataArray queue_history;
	pthread_mutex_t mutex;
	struct deque audio_data[MAX_AUDIO_CHANNELS];
	struct deque audio_frames;
	struct deque audio_timestamps;
	size_t num_channels;
	pthread_mutex_t audio_mutex;
#pragma endregion
} PlaylistContext;

// class PlaylistData {
// 	public:
// 		QLabel *label;
// 		e_MediaStringifyTYPE media_stringify_type;
// 		const MediaData *media_data;
// 		explicit MediaWidget(const MediaData *media_data,
// 					 e_MediaStringifyTYPE media_stringify_type = MEDIA_STRINGIFY_TYPE_FILENAME,
// 					 QWidget *parent = nullptr);
// 		void update_media_data(e_MediaStringifyTYPE *media_stringify_type = nullptr);
// 		void remove_widget(bool delete_later = true);
// 	};

typedef struct PlaylistData {
	PlaylisQueueWidget *playlist_widget;
	PlaylisQueueWidget *param_playlist_widget;
	PlaylistContext *playlist_context;
} PlaylistData;

#endif // PLAYLIST_SOURCE_TYPES_HPP