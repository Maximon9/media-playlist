#ifndef PLAYLIST_SOURCE_TYPES_HPP
#define PLAYLIST_SOURCE_TYPES_HPP

#include <util/threading.h>
#include <util/deque.h>
#include "../include/utils/utils.hpp"
#include "../include/utils/enum-utils.hpp"

typedef struct {
#pragma region Public
	int queue_list_size;
	MediaFileDataArray all_media;
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
	std::string name;
	bool all_media_initialized;
	obs_data_t *media_source_settings;
	MediaFileDataArray queue;
	MediaFileDataArray previous_queue;
	pthread_mutex_t mutex;
	struct deque audio_data[MAX_AUDIO_CHANNELS];
	struct deque audio_frames;
	struct deque audio_timestamps;
	size_t num_channels;
	pthread_mutex_t audio_mutex;
#pragma endregion
} PlaylistData;

#endif // PLAYLIST_SOURCE_TYPES_HPP