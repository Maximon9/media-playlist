#ifndef MEDIA_FILE_ARRAY_UTILS_H
#define MEDIA_FILE_ARRAY_UTILS_H

#include <obs-module.h>
#include <plugin-support.h>
#include <util/dstr.h>
#include <filesystem>
#include "../types/media-file-data-types.hpp"
#include "../utils/enum-utils.hpp"

// using namespace std;
namespace fs = std::filesystem;

static const char *media_filter =
	" (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm *.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";
static const char *video_filter = " (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm);;";
static const char *audio_filter = " (*.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";

void push_media_back(MediaFileDataArray *media_array, const std::string path);
void push_media_front(MediaFileDataArray *media_array, const std::string path);

void push_media_at(MediaFileDataArray *media_array, const std::string path, size_t index);

void push_media_media_file_at(MediaFileDataArray *media_array, MediaFileData *entry, size_t index);

void pop_media_at(MediaFileDataArray *media_array, size_t index);

MediaFileData create_media_file_data_from_path(std::string path, size_t index);

// MediaFileData create_media_file_data_from_path_and_file_name(const std::string path, const std::string file_name,
// 							     size_t index);

MediaFileData create_media_file_data_with_all_info(const std::string path, const std::string filename,
						   const std::string name, const std::string ext, size_t index);

std::string obs_array_to_string(obs_data_array_t *array);

// Returns true if extension is valid
bool valid_extension(const std::string *ext);

// Turns an obs_data_array_t into a MediaFileDataArray
// void obs_data_media_array_retain(MediaFileDataArray *media_file_data_array, obs_data_array_t *obs_playlist);

// Turns the media array into a string
const char *stringify_media_array(const MediaFileDataArray *media_array, size_t threshold, const std::string *indent,
				  e_MediaStringifyTYPE media_stringify_type);

const char *stringify_media_queue_array(const MediaFileDataArray *media_array, int queue_limit,
					const std::string indent, e_MediaStringifyTYPE media_stringify_type);

// Logs the media array in the obs log files
void obs_log_media_array(int log_level, std::string format, const MediaFileDataArray *media_array, size_t threshold,
			 const std::string indent, e_MediaStringifyTYPE media_stringify_type);

bool compare_media_file_data(const MediaFileData *data_1, const MediaFileData *data_2);

bool compare_media_file_data_arrays(const MediaFileDataArray *array_1, const MediaFileDataArray *array_2);

#endif // MEDIA_FILE_ARRAY_UTILS_H
