#ifndef PLAYLIST_WIDGET_HPP
#define PLAYLIST_WIDGET_HPP

#include <QEventLoop>
#include <QScrollArea>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../include/types/playlist-source-types.hpp"

typedef struct PlaylistData PlaylistData;

class PlaylistWidget : public QWidget {
private:
	QVBoxLayout *playlist_layout;
	QPushButton *toggleButton;
	QWidget *mediaContainer;
	QVBoxLayout *mediaLayout;
	QHBoxLayout *buttonLayout;
	bool expanded;

#pragma region Main Widget Params
	bool is_main_widget;
	QVBoxLayout *layout;
	QScrollArea *scrollArea;
	QWidget *contentWidget;
	QVBoxLayout *contentLayout;
#pragma endregion

public:
	const PlaylistData *playlist_data;
	explicit PlaylistWidget(const PlaylistData *playlist, QWidget *parent = nullptr, bool is_main_widget = false);
	void toggleMediaVisibility();
	void update_playlist_name();
	void update_playlist_data();
	void remove_widget();
	// void create_media_widget(MediaData *media_data, std::function<void(MediaWidget *)> callback);
	MediaWidget *create_media_widget(MediaData *media_data);
	void add_media_widget(MediaWidget *mediaWidget);
};
#endif // PLAYLIST_WIDGET_HPP