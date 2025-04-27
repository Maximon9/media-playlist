#ifndef UTILS_H
#define UTILS_H

#include <obs-module.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <random>
#include "../types/media-vault-source-types.hpp"
#include <obs-frontend-api.h>

// std::string  *concat_mem_string(std::string  *_Destination, const std::string  *_Source);
typedef struct MediaVaultContext MediaVaultContext;

void handle_stretch_mode(MediaVaultContext *media_vault_context);

void handle_keep(MediaVaultContext *media_vault_context, vec2 *end_size);

void handle_keep_aspect(MediaVaultContext *media_vault_context, vec2 *end_size);

void handle_keep_aspect_covered(MediaVaultContext *media_vault_context, vec2 *end_size);

void handle_scale(MediaVaultContext *media_vault_context, vec2 *end_size);

void position_top_left(MediaVaultContext *media_vault_context);

void position_center(MediaVaultContext *media_vault_context, vec2 *end_size);

size_t get_random_size_t(size_t min, size_t max);

std::string screaming_snake_case_to_title_case(const std::string &input, size_t rem_word_count);

void add_enums_to_property_list(obs_property_t *property, const char *Enum[], int word_count_to_remove);

#endif // UTILS_H
