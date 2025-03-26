#ifndef PLAYLIST_WIDGET_HPP
#define PLAYLIST_WIDGET_HPP

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
	bool expanded;

public:
	QVBoxLayout *mediaLayout;
	const PlaylistData *playlist_data;
	explicit PlaylistWidget(const PlaylistData *playlist, QWidget *parent = nullptr);
	void toggleMediaVisibility();
	void update_playlist_name();
	void update_playlist_data();
	void remove_widget();
};
#endif // PLAYLIST_WIDGET_HPP