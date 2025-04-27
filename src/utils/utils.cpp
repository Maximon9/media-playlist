#pragma region Main

#include "../include/utils/utils.hpp"

void handle_stretch_mode(PlaylistContext *playlist_context)
{
	if (!playlist_context || !playlist_context->media_source || !playlist_context->source_scene_item)
		return;
	vec2 end_size{};
	switch (playlist_context->stretch_mode) {
	case STRETCH_KEEP:
		handle_keep(playlist_context, &end_size);
		position_top_left(playlist_context);
		break;
	case STRETCH_KEEP_CENTERED:
		handle_keep(playlist_context, &end_size);
		position_center(playlist_context, &end_size);
		break;
	case STRETCH_KEEP_ASPECT:
		handle_keep_aspect(playlist_context, &end_size);
		position_top_left(playlist_context);
		break;
	case STRETCH_KEEP_ASPECT_CENTERED:
		handle_keep_aspect(playlist_context, &end_size);
		position_center(playlist_context, &end_size);
		break;
	case STRETCH_KEEP_ASPECT_COVERED:
		handle_keep_aspect_covered(playlist_context, &end_size);
		position_center(playlist_context, &end_size);
		break;
	case STRETCH_SCALE:
		handle_scale(playlist_context, &end_size);
		position_center(playlist_context, &end_size);
		break;
	default:
		break;
	}
}

void handle_keep(PlaylistContext *playlist_context, vec2 *end_size)
{
	// Get OBS base canvas size
	video_t *video = obs_get_video();
	uint32_t canvas_width = video_output_get_width(video);
	uint32_t canvas_height = video_output_get_height(video);

	// Get media source size
	end_size->x = obs_source_get_width(playlist_context->media_source);
	end_size->y = obs_source_get_height(playlist_context->media_source);

	// Set the new scale
	struct vec2 scale = {1, 1};
	obs_sceneitem_set_scale(playlist_context->source_scene_item, &scale);
}

void handle_keep_aspect(PlaylistContext *playlist_context, vec2 *end_size)
{
	// Get OBS base canvas size
	video_t *video = obs_get_video();
	uint32_t canvas_width = video_output_get_width(video);
	uint32_t canvas_height = video_output_get_height(video);

	// Get media source size
	end_size->x = obs_source_get_width(playlist_context->media_source);
	end_size->y = obs_source_get_height(playlist_context->media_source);

	// Calculate scale factor to fit inside canvas while keeping aspect ratio
	float scale_x = canvas_width / end_size->x;
	float scale_y = canvas_height / end_size->y;
	float scale_factor = fmin(scale_x, scale_y);

	end_size->x *= scale_factor;
	end_size->y *= scale_factor;

	// Set the new scale
	struct vec2 scale = {scale_factor, scale_factor};
	obs_sceneitem_set_scale(playlist_context->source_scene_item, &scale);
}

void handle_keep_aspect_covered(PlaylistContext *playlist_context, vec2 *end_size)
{
	// Get OBS base canvas size
	video_t *video = obs_get_video();
	uint32_t canvas_width = video_output_get_width(video);
	uint32_t canvas_height = video_output_get_height(video);

	// Get media source size
	end_size->x = obs_source_get_width(playlist_context->media_source);
	end_size->y = obs_source_get_height(playlist_context->media_source);

	// Calculate scale factor to fit inside canvas while keeping aspect ratio
	float scale_x = canvas_width / end_size->x;
	float scale_y = canvas_height / end_size->y;
	float scale_factor = fmax(scale_x, scale_y);

	end_size->x *= scale_factor;
	end_size->y *= scale_factor;

	// Set the new scale
	struct vec2 scale = {scale_factor, scale_factor};
	obs_sceneitem_set_scale(playlist_context->source_scene_item, &scale);
}

void handle_scale(PlaylistContext *playlist_context, vec2 *end_size)
{
	// Get OBS base canvas size
	video_t *video = obs_get_video();
	uint32_t canvas_width = video_output_get_width(video);
	uint32_t canvas_height = video_output_get_height(video);

	// Get media source size
	end_size->x = obs_source_get_width(playlist_context->media_source);
	end_size->y = obs_source_get_height(playlist_context->media_source);

	// Calculate scale factor to fit inside canvas while keeping aspect ratio
	float scale_x = canvas_width / end_size->x;
	float scale_y = canvas_height / end_size->y;

	end_size->x *= scale_x;
	end_size->y *= scale_y;

	// Set the new scale
	struct vec2 scale = {scale_x, scale_y};
	obs_sceneitem_set_scale(playlist_context->source_scene_item, &scale);
}

void position_top_left(PlaylistContext *playlist_context)
{
	// Get OBS base canvas size
	// video_t *video = obs_get_video();
	// uint32_t canvas_width = video_output_get_width(video);
	// uint32_t canvas_height = video_output_get_height(video);

	// // Get media source size
	// uint32_t media_width = obs_source_get_width(playlist_context->media_source);
	// uint32_t media_height = obs_source_get_height(playlist_context->media_source);

	// if (media_width == 0 || media_height == 0)
	// return; // Avoid division by zero

	// Calculate scale factor to fit inside canvas while keeping aspect ratio
	struct vec2 pos = {0, 0};
	obs_sceneitem_set_pos(playlist_context->source_scene_item, &pos);
}

void position_center(PlaylistContext *playlist_context, vec2 *end_size)
{
	// Get OBS base canvas size
	video_t *video = obs_get_video();
	uint32_t canvas_width = video_output_get_width(video);
	uint32_t canvas_height = video_output_get_height(video);

	// Calculate position to center (no scaling, only move)
	float pos_x = (((float)canvas_width) - end_size->x) / 2.0f;
	float pos_y = (((float)canvas_height) - end_size->y) / 2.0f;

	struct vec2 pos = {pos_x, pos_y};

	// Set position
	obs_sceneitem_set_pos(playlist_context->source_scene_item, &pos);
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