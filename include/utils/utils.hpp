#ifndef UTILS_H
#define UTILS_H

#include <obs-module.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <random>
#include "../types/playlist-source-types.hpp"
#include <obs-frontend-api.h>

// std::string  *concat_mem_string(std::string  *_Destination, const std::string  *_Source);
typedef struct PlaylistContext PlaylistContext;

void scale_media_source_to_fit(PlaylistContext *playlist_context);

size_t get_random_size_t(size_t min, size_t max);

std::string screaming_snake_case_to_title_case(const std::string &input);

void add_enums_to_property_list(obs_property_t *property, const char *Enum[], int word_count_to_remove);

#endif // UTILS_H
