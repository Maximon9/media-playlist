#ifndef PLAYLIST_WIDGET_HPP
#define PLAYLIST_WIDGET_HPP

#include <QEventLoop>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../include/types/playlist-source-types.hpp"

typedef struct PlaylistData PlaylistData;

class PlaylistWidget : public QWidget {
private:
	QVBoxLayout *layout;
	QPushButton *toggleButton;
	QWidget *mediaContainer;
	// QWidget *topContainer;
	// QVBoxLayout *topLayout;
	QVBoxLayout *mediaLayout;
	QHBoxLayout *buttonLayout;
	bool expanded;

public:
	const PlaylistData *playlist_data;
	explicit PlaylistWidget(const PlaylistData *playlist, QWidget *parent = nullptr);
	void toggleMediaVisibility();
	void update_playlist_name();
	void update_playlist_data();
	void remove_widget();
	// void create_media_widget(MediaData *media_data, std::function<void(MediaWidget *)> callback);
	MediaWidget *create_media_widget(MediaData *media_data);
	void add_media_widget(MediaWidget *mediaWidget);
};
#endif // PLAYLIST_WIDGET_HPP