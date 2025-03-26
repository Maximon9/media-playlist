#ifndef MEDIA_FILE_ARRAY_UTILS_H
#define MEDIA_FILE_ARRAY_UTILS_H

#include <obs-module.h>
#include <plugin-support.h>
#include <util/dstr.h>
#include <filesystem>
#include "../types/media-file-data-types.hpp"
#include <../../include/types/playlist-source-types.hpp>
#include "../utils/enum-utils.hpp"

// using namespace std;
namespace fs = std::filesystem;

static const char *media_filter =
	" (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm *.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";
static const char *video_filter = " (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm);;";
static const char *audio_filter = " (*.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";

void push_media_back(MediaDataArray *media_array, const std::string path);
void push_media_front(MediaDataArray *media_array, const std::string path);

void push_media_at(MediaDataArray *media_array, const std::string path, size_t index);

void push_media_media_file_at(MediaDataArray *media_array, MediaData *entry, size_t index);

void pop_media_at(MediaDataArray *media_array, size_t index);

MediaData load_media_data_from_path(std::string path, size_t index);

MediaData construct_complete_media_data(const std::string path, const std::string filename, const std::string name,
					const std::string ext, size_t index);

void push_queue_media_back(QueueMediaDataArray *media_array, const std::string path, PlaylistData *playlist_data);

void push_queue_media_front(QueueMediaDataArray *media_array, const std::string path, PlaylistData *playlist_data);

void push_queue_media_at(QueueMediaDataArray *media_array, const std::string path, size_t index,
			 PlaylistData *playlist_data);

void pop_queue_media_back(QueueMediaDataArray *media_array);

void pop_queue_media_front(QueueMediaDataArray *media_array);

void pop_queue_media_at(QueueMediaDataArray *media_array, size_t index);

QueueMediaData load_queue_media_data_from_path(std::string path, size_t index, PlaylistData *playlist_data);

QueueMediaData construct_complete_queue_media_data(const std::string path, const std::string filename,
						   const std::string name, const std::string ext, size_t index,
						   MediaWidget *media_widget, PlaylistData *playlist_data);

std::string obs_array_to_string(obs_data_array_t *array);

// Returns true if extension is valid
bool valid_extension(const std::string *ext);

// Turns an obs_data_array_t into a MediaDataArray
// void obs_data_media_array_retain(MediaDataArray *media_file_data_array, obs_data_array_t *obs_playlist);

// Turns the media array into a string
const char *stringify_media_array(const MediaDataArray *media_array, size_t threshold, const std::string *indent,
				  e_MediaStringifyTYPE media_stringify_type);

const char *stringify_media_queue_array(const MediaDataArray *media_array, int queue_limit, const std::string indent,
					e_MediaStringifyTYPE media_stringify_type);

// Logs the media array in the obs log files
void obs_log_media_array(int log_level, std::string format, const MediaDataArray *media_array, size_t threshold,
			 const std::string indent, e_MediaStringifyTYPE media_stringify_type);

bool compare_media_file_data(const MediaData *data_1, const MediaData *data_2);

bool compare_media_file_data_arrays(const MediaDataArray *array_1, const MediaDataArray *array_2);

#endif // MEDIA_FILE_ARRAY_UTILS_H
