#ifndef UTILS_H
#define UTILS_H

#include <obs-module.h>
#include <string>
#include <sstream>

// std::string  *concat_mem_string(std::string  *_Destination, const std::string  *_Source);

std::string screaming_snake_case_to_title_case(const std::string &input);

void add_enums_to_property_list(obs_property_t *property, const char *Enum[], int word_count_to_remove);

#endif // UTILS_H
