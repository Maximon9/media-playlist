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

typedef std::deque<PlaylistData *> PlaylistDatas;

class PlaylistQueueViewer : public QWidget {
private:
	QVBoxLayout *layout;
	QScrollArea *scrollArea;
	QWidget *contentWidget;

public:
	QVBoxLayout *contentLayout;
	PlaylistDatas playlist_datas;
	explicit PlaylistQueueViewer(QWidget *parent = nullptr);
	void updatePlaylists();
};

extern PlaylistQueueViewer *playlist_queue_viewer;

/*
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <deque>
#include "../include/source-types/playlist-source-types.hpp"

typedef std::deque<PlaylistData *> PlaylistDatas;

class PlaylistQueueViewer : public QWidget {
private:
	QVBoxLayout *layout;
	QScrollArea *scrollArea;
	QWidget *contentWidget;
	QVBoxLayout *contentLayout;

public:
	PlaylistDatas *playlist_datas;
	explicit PlaylistQueueViewer(QWidget *parent = nullptr);
	void updatePlaylists();
};

static PlaylistQueueViewer *playlist_queue_viewer;
*/

#endif // PLAYLIST_QUEUE_VIEWER_HPP