#ifndef MEDIA_FILE_ARRAY_UTILS_H
#define MEDIA_FILE_ARRAY_UTILS_H

#include <obs-module.h>
#include <plugin-support.h>
#include <util/dstr.h>
#include <filesystem>
#include "../types/media-file-data-types.hpp"
#include <../../include/types/playlist-source-types.hpp>
#include "../utils/enum-utils.hpp"
#include <iostream>
#include <algorithm>
#include <random>

// using namespace std;
namespace fs = std::filesystem;

static const char *media_filter =
	" (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm *.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";
static const char *video_filter = " (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm);;";
static const char *audio_filter = " (*.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";

void push_media_back(MediaList *media_array, const std::string path);

void push_media_front(MediaList *media_array, const std::string path);

void push_media_at(MediaList *media_array, const std::string path, size_t index);

void push_media_data_back(MediaList *media_array, MediaContext media_data);

void push_media_data_front(MediaList *media_array, MediaContext media_data);

void push_media_data_at(MediaList *media_array, MediaContext media_data, size_t index);

void pop_media_at(MediaList *media_array, size_t index);

MediaContext init_media_data_from_path(std::string path, size_t index);

MediaContext init_media_data(const std::string path, const std::string filename, const std::string name,
			     const std::string ext, size_t index);

MediaContext init_media_data_from_media_data(const MediaContext media_data);

void push_queue_media_path_back(QueuedMedia *media_array, const std::string path, PlaylistData *playlist_data);

void push_queue_media_path_front(QueuedMedia *media_array, const std::string path, PlaylistData *playlist_data);

void push_queue_media_path_at(QueuedMedia *media_array, const std::string path, size_t index,
			      PlaylistData *playlist_data);

void push_queue_media_data_back(QueuedMedia *media_array, MediaContext media_data, PlaylistData *playlist_data);

void push_queue_media_data_front(QueuedMedia *media_array, MediaContext media_data, PlaylistData *playlist_data);

void push_queue_media_data_at(QueuedMedia *media_array, MediaContext media_data, size_t index,
			      PlaylistData *playlist_data);

void push_queue_media_at(QueuedMedia *media_array, QueueMedia new_entry, size_t index);

QueueMedia pop_queue_media_back(QueuedMedia *media_array, bool erase_widget = true);

QueueMedia pop_queue_media_front(QueuedMedia *media_array, bool erase_widget = true);

QueueMedia pop_queue_media_at(QueuedMedia *media_array, size_t index, bool erase_widget = true);

void clear_queue(QueuedMedia *media_array);

void shuffle_queue(QueuedMedia *media_array, PlaylistData *playlist_data);

void init_widgets(QueueMedia entry, size_t index, PlaylistData *playlist_data, MediaWidget *media_widget = nullptr,
		  MediaWidget *param_media_widget = nullptr);

QueueMedia init_queue_media_data_from_path(std::string path, size_t widget_index, size_t index,
					   PlaylistData *playlist_data, MediaWidget *media_widget = nullptr,
					   MediaWidget *param_media_widget = nullptr);

QueueMedia init_queue_media_data(const std::string path, const std::string filename, const std::string name,
				 const std::string ext, size_t widget_index, size_t index, PlaylistData *playlist_data,
				 MediaWidget *media_widget = nullptr, MediaWidget *param_media_widget = nullptr);

QueueMedia init_queue_media_data_from_media_data(MediaContext media_data, size_t widget_index,
						 PlaylistData *playlist_data, MediaWidget *media_widget = nullptr,
						 MediaWidget *param_media_widget = nullptr);

std::string obs_array_to_string(obs_data_array_t *array);

// Returns true if extension is valid
bool valid_extension(const std::string *ext);

// Turns an obs_data_array_t into a MediaList
// void obs_data_media_array_retain(MediaList *media_file_data_array, obs_data_array_t *obs_playlist);

// Turns the media array into a string
const char *stringify_media_array(const MediaList *media_array, size_t threshold, const std::string indent,
				  e_MediaStringifyTYPE media_stringify_type);

const char *stringify_queue_media_array(const QueuedMedia *media_array, int queue_limit, const std::string indent,
					e_MediaStringifyTYPE media_stringify_type);

void obs_log_media_array(int log_level, std::string format, const MediaList *media_array, size_t threshold,
			 const std::string indent, e_MediaStringifyTYPE media_stringify_type);

void obs_log_queue_media_array(int log_level, std::string format, const QueuedMedia *media_array, int queue_limit,
			       const std::string indent, e_MediaStringifyTYPE media_stringify_type);

bool compare_media_file_data(const MediaContext *data_1, const MediaContext *data_2);

bool compare_media_file_data_arrays(const MediaList *array_1, const MediaList *array_2);

#endif // MEDIA_FILE_ARRAY_UTILS_H
