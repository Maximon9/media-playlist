#ifndef PLAYLIST_QUEUE_VIEWER_HPP
#define PLAYLIST_QUEUE_VIEWER_HPP

#include <obs-module.h>
#include <plugin-support.h>
#include <deque>
#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include "../include/types/playlist-source-types.hpp"

typedef std::deque<PlaylistWidgetData *> PlaylistWidgetDatas;

class MultiPlaylistQueueViewer : public QWidget {
private:
	QVBoxLayout *layout;
	QScrollArea *scrollArea;
	QWidget *contentWidget;
	QVBoxLayout *contentLayout;

public:
	PlaylistWidgetDatas playlist_datas;
	explicit MultiPlaylistQueueViewer(QWidget *parent = nullptr);
	void addPlaylistWidget(PlaylistWidget *playlist_widget);
	void updatePlaylists();
};

extern MultiPlaylistQueueViewer *playlist_queue_viewer;

#endif // PLAYLIST_QUEUE_VIEWER_HPP