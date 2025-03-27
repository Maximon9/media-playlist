#pragma region Main

#include "../../include/qt-classes/playlist-queue-viewer.hpp"
#include "../../include/qt-classes/playlist-widget.hpp"

/* PlaylistQueueViewer::PlaylistQueueViewer(QWidget *parent) : QWidget(parent)
{
	this->playlist_datas = {};

	this->layout = new QVBoxLayout(this);

	QLabel *label = new QLabel("Hi, this is my custom dock", this);

	layout->addWidget(label);

	setLayout(this->layout);
} */

PlaylistQueueViewer::PlaylistQueueViewer(QWidget *parent) : QWidget(parent)
{
	moveToThread(parent->thread());
	this->playlist_datas = {};
	layout = new QVBoxLayout(this);

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
	// Add new playlists
	for (size_t i = 0; i < this->playlist_datas.size(); i++) {
		const PlaylistData *playlist_data = this->playlist_datas[i];
		if (playlist_data->playlist_widget == nullptr) {
			continue;
		}
		playlist_data->playlist_widget->update_playlist_data();
	}
}

PlaylistQueueViewer *playlist_queue_viewer = nullptr;

#pragma endregion