#ifndef PLAYLIST_QUEUE_VIEWER_HPP
#define PLAYLIST_QUEUE_VIEWER_HPP

#include <QScrollArea>
#include "../include/types/playlist-source-types.hpp"

class PlaylistQueueViewer : public PlaylistWidget {
private:
	QVBoxLayout *layout;
	QScrollArea *scrollArea;
	QWidget *contentWidget;
	QVBoxLayout *contentLayout;

public:
	const PlaylistData *playlist_data;
	explicit PlaylistQueueViewer(const PlaylistData *playlist, QWidget *parent = nullptr);
};

#endif // PLAYLIST_QUEUE_VIEWER_HPP