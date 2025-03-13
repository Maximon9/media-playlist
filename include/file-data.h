#pragma once

#include <util/darray.h>

struct MediaFileData {
	char *path;
	char *filename; // filename with ext, ONLY for folder item checking
	char *id;
	bool is_url;
	bool is_folder;
	DARRAY(struct MediaFileData) folder_items;
	struct MediaFileData *parent;
	const char *parent_id; // for folder items
	size_t index;          // makes it easier to switch back to non-shuffle mode
};