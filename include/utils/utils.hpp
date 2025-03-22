#ifndef UTILS_H
#define UTILS_H

#include <obs-module.h>
#include <plugin-support.h>
// #include <util/darray.h>
#include <util/dstr.h>
#include <deque>
// #include <stdio.h>
// #include <stdlib.h>
// #include <ctype.h>
#include <string.h>
#include <util/platform.h>
#include "../include/utils/enum-utils.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

// using namespace std;
namespace fs = std::filesystem;
using string = std::string;

auto min = [](auto a, auto b) {
	return std::min(a, b);
};
auto swap = [](auto a, auto b) {
	return std::swap(a, b);
};

static const char *media_filter =
	" (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm *.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";
static const char *video_filter = " (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm);;";
static const char *audio_filter = " (*.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";

typedef struct {
	string path;
	string filename;
	string name;
	string ext;
	size_t index;
} MediaFileData;

// typedef struct {
// 	MediaFileData *data;
// 	size_t size;
// 	size_t capacity;
// } MediaFileDataArray;
typedef std::deque<MediaFileData> MediaFileDataArray;
// typedef vector<MediaFileData> MediaFileDataArray;

typedef DARRAY(size_t) SizeTArray;

// typedef DARRAY(MediaFileData) MediaFileDataArray;

#pragma region Media Array Utils

/* void init_media_array(MediaFileDataArray *media_array, size_t initial_capacity);

void push_media_back(MediaFileDataArray *media_array, const string *path);

void push_media_front(MediaFileDataArray *media_array, const string *path);

void push_media_at(MediaFileDataArray *media_array, const string *path, size_t index);

void push_media_file_data_back(MediaFileDataArray *media_array, MediaFileData media_file_data);

void push_media_file_data_front(MediaFileDataArray *media_array, MediaFileData media_file_data);

void push_media_file_data_at(MediaFileDataArray *media_array, MediaFileData media_file_data, size_t index);

void pop_media_back(MediaFileDataArray *media_array);

void pop_media_front(MediaFileDataArray *media_array);

void pop_media_at(MediaFileDataArray *media_array, size_t index);

void move_media_at(MediaFileDataArray *media_array, size_t from, size_t to);

// Function to get a string by index
const MediaFileData *get_media(const MediaFileDataArray *media_array, size_t index);

void move_media_array(MediaFileDataArray *destination, MediaFileDataArray *source);

void clear_media_array(MediaFileDataArray *media_array);

// Function to free the dynamic string array
void free_media_array(MediaFileDataArray *media_array); */

MediaFileData create_media_file_data_from_path(const string *path, size_t index);

MediaFileData create_media_file_data_from_path_and_file_name(const string path, const string file_name, size_t index);
MediaFileData create_media_file_data_with_all_info(const string path, const string filename, const string name,
						   const string ext, size_t index);

string obs_array_to_string(obs_data_array_t *array);

// Returns true if extension is valid
bool valid_extension(const string *ext);

// Turns an obs_data_array_t into a MediaFileDataArray
void obs_data_media_array_retain(MediaFileDataArray *media_file_data_array, obs_data_array_t *obs_playlist);

// Turns the media array into a string
string stringify_media_array(const MediaFileDataArray *media_array, size_t threshold, const string *indent,
			     e_MediaStringifyTYPE media_stringify_type);

string stringify_media_queue_array(const MediaFileDataArray *media_array, int queue_limit, const string indent,
				   e_MediaStringifyTYPE media_stringify_type);

// Logs the media array in the obs log files
void obs_log_media_array(int log_level, string format, const MediaFileDataArray *media_array, size_t threshold,
			 const string indent, e_MediaStringifyTYPE media_stringify_type);

bool compare_media_file_data(const MediaFileData *data_1, const MediaFileData *data_2);

bool compare_media_file_data_arrays(const MediaFileDataArray *array_1, const MediaFileDataArray *array_2);

#pragma endregion

#pragma region Utils

// string *concat_mem_string(string *_Destination, const string *_Source);

string screaming_snake_case_to_title_case(const string *name);

string *remove_screaming_case_words_front(const string *name, size_t count);

void add_enums_to_property_list(obs_property_t *property, const char *Enum[], int word_count_to_remove);

#pragma endregion

#endif // UTILS_H
