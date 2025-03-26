#include "../include/qt-classes/playlist-queue-viewer.hpp"

PlaylistQueueViewer::PlaylistQueueViewer(QWidget *parent) : QWidget(parent)
{
	this->playlist_datas = {};

	this->layout = new QVBoxLayout(this);

	QLabel *label = new QLabel("Hi, this is my custom dock", this);

	layout->addWidget(label);

	setLayout(this->layout);
}

/*
#include "../../include/qt-classes/playlist-queue-viewer.hpp"
#include "../../include/qt-classes/playlist-widget.hpp"

// CustomProperties::CustomProperties() : QDialog() {}

PlaylistQueueViewer::PlaylistQueueViewer(QWidget *parent) : QWidget(parent)
{
	this->playlist_datas = {};
	this->layout = new QVBoxLayout(this);

	// Scroll area setup
	scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);

	// Container inside scroll area
	contentWidget = new QWidget();
	contentLayout = new QVBoxLayout(contentWidget);
	contentWidget->setLayout(contentLayout);
	scrollArea->setWidget(contentWidget);

	layout->addWidget(scrollArea);
	setLayout(layout);
}

void PlaylistQueueViewer::updatePlaylists()
{
	QLayoutItem *child;
	while ((child = contentLayout->takeAt(0)) != nullptr) {
		delete child->widget(); // Delete each PlaylistWidget
		delete child;
	}

	// Add new playlists
	for (const PlaylistData *playlist : *this->playlist_datas) {
		PlaylistWidget *playlistWidget = new PlaylistWidget(playlist, this);
		contentLayout->addWidget(playlistWidget);
	}
}

#endif // CUSTOM_PROPERTIES_DIALOG_HPP */