#ifndef PLAYLIST_WIDGET_HPP
#define PLAYLIST_WIDGET_HPP

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../include/types/playlist-source-types.hpp"

class PlaylistWidget : public QWidget {
private:
	QVBoxLayout *layout;
	QPushButton *toggleButton;
	QWidget *mediaContainer;
	QVBoxLayout *mediaLayout;
	bool expanded;

public:
	explicit PlaylistWidget(const PlaylistData *playlist, QWidget *parent = nullptr);
	void toggleMediaVisibility();
};
#endif // PLAYLIST_WIDGET_HPP