#pragma region Main

#include "../include/utils/utils.hpp"

std::string screaming_snake_case_to_title_case(const std::string &input, size_t rem_word_count)
{
	std::stringstream ss(input);
	std::string word, result;
	int removed = 0;
	bool capitalize = true;

	for (char c : input) {
		if (c == '_') {
			if (removed < rem_word_count) {
				removed++;
				continue; // Skip underscore and the next word
			}
			result += ' '; // Replace underscore with space
			capitalize = true;
		} else {
			if (removed < rem_word_count)
				continue; // Skip characters of the removed words
			result += capitalize ? std::toupper(c) : std::tolower(c);
			capitalize = false;
		}
	}

	return result;
}

void add_enums_to_property_list(obs_property_t *property, const char *Enum[], int word_count_to_remove)
{
	long long i = 0;
	std::string name = Enum[i];
	while (name != "") {
		std::string display_name = screaming_snake_case_to_title_case(name, word_count_to_remove);
		obs_property_list_add_int(property, display_name.c_str(), i);
		i++;
		name = Enum[i];
	}
}

#pragma endregion