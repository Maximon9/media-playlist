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

void handle_stretch_mode(PlaylistContext *playlist_context);

void handle_keep(PlaylistContext *playlist_context, vec2 *end_size);

void handle_keep_aspect(PlaylistContext *playlist_context, vec2 *end_size);

void handle_keep_aspect_covered(PlaylistContext *playlist_context, vec2 *end_size);

void handle_scale(PlaylistContext *playlist_context, vec2 *end_size);

void position_top_left(PlaylistContext *playlist_context);

void position_center(PlaylistContext *playlist_context, vec2 *end_size);

size_t get_random_size_t(size_t min, size_t max);

std::string screaming_snake_case_to_title_case(const std::string &input, size_t rem_word_count);

void add_enums_to_property_list(obs_property_t *property, const char *Enum[], int word_count_to_remove);

#endif // UTILS_H
