#ifndef MEDIA_FILE_DATA_TYPES_HPP
#define MEDIA_FILE_DATA_TYPES_HPP

#include <string>
#include <deque>
#include "../qt-classes/media-widget.hpp"

typedef struct {
	MediaWidget *media_widget;
	std::string path;
	std::string filename;
	std::string name;
	std::string ext;
	size_t index;
} MediaFileData;

typedef std::deque<MediaFileData> MediaFileDataArray;

#endif MEDIA_FILE_DATA_TYPES_HPP