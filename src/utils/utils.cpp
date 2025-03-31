#pragma region Main

#include "../include/utils/utils.hpp"

void scale_media_source_to_fit(PlaylistContext *playlist_context)
{
	if (!playlist_context || !playlist_context->media_source || !playlist_context->source_scene_item)
		return;

	// Get OBS base canvas size
	video_t *video = obs_get_video();
	uint32_t canvas_width = video_output_get_width(video);
	uint32_t canvas_height = video_output_get_height(video);

	// Get media source size
	uint32_t media_width = obs_source_get_width(playlist_context->media_source);
	uint32_t media_height = obs_source_get_height(playlist_context->media_source);

	if (media_width == 0 || media_height == 0)
		return; // Avoid division by zero

	// Calculate scale factor to fit inside canvas while keeping aspect ratio
	float scale_x = (float)canvas_width / media_width;
	float scale_y = (float)canvas_height / media_height;
	float scale_factor = fmin(scale_x, scale_y);

	// Set the new scale
	struct vec2 scale = {scale_factor, scale_factor};
	obs_sceneitem_set_scale(playlist_context->source_scene_item, &scale);
}

size_t get_random_size_t(size_t min, size_t max)
{
	std::random_device rd;
	std::mt19937 gen(rd()); // Mersenne Twister RNG
	std::uniform_int_distribution<size_t> dist(min, max);
	return dist(gen);
}

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