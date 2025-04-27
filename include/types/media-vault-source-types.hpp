#ifndef PLAYLIST_SOURCE_TYPES_HPP
#define PLAYLIST_SOURCE_TYPES_HPP

#include <obs-module.h>
#include <plugin-support.h>
#include <util/threading.h>
#include <util/deque.h>
#include "./media-file-data-types.hpp"
#include "../include/utils/enum-utils.hpp"
#include "../include/qt-classes/media-vault-queue-widget.hpp"

class MediaVaultQueueWidget;

typedef struct MediaVaultContext {
#pragma region Exported
	e_StretchMode stretch_mode;
	MediaList all_media;
	bool shuffle_queue;
	e_StartBehavior start_behavior;
	e_EndBehavior end_behavior;
	int loop_index;
	bool infinite;
	int max_loop_count;
	e_LoopEndBehavior loop_end_behavior;
	int media_history_limit;
	bool debug;
#pragma endregion
#pragma region Private
	obs_source_t *source;
	obs_source_t *media_source;
	obs_data_t *media_source_settings;
	obs_sceneitem_t *source_scene_item;
	bool restarting_media_source;
	int loop_count;
	obs_media_state state;
	std::string name;
	bool all_media_initialized;
	QueuedMedia queue;
	MediaList queue_history;
	pthread_mutex_t mutex;
	struct deque audio_data[MAX_AUDIO_CHANNELS];
	struct deque audio_frames;
	struct deque audio_timestamps;
	size_t num_channels;
	pthread_mutex_t audio_mutex;
#pragma endregion
} MediaVaultContext;

// class MediaVaultData {
// 	public:
// 		QLabel *label;
// 		e_MediaStringifyTYPE media_stringify_type;
// 		const MediaContext *media_context;
// 		explicit MediaWidget(const MediaContext *media_context,
// 					 e_MediaStringifyTYPE media_stringify_type = MEDIA_STRINGIFY_TYPE_FILENAME,
// 					 QWidget *parent = nullptr);
// 		void update_media_data(e_MediaStringifyTYPE *media_stringify_type = nullptr);
// 		void remove_widget(bool delete_later = true);
// 	};

typedef struct MediaVaultData {
	MediaVaultQueueWidget *media_vault_widget;
	MediaVaultQueueWidget *param_media_vault_widget;
	MediaVaultContext *media_vault_context;
} MediaVaultData;

#endif // PLAYLIST_SOURCE_TYPES_HPP