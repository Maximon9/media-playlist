#pragma region Main

#include "../../include/qt-classes/playlist-widget.hpp"
#include "../../include/qt-classes/media-widget.hpp"

PlaylistWidget::PlaylistWidget(const PlaylistData *playlist, QWidget *parent) : QWidget(parent), expanded(false)
{
	this->playlist = playlist;

	layout = new QVBoxLayout(this);
	toggleButton = new QPushButton(QString::fromStdString(playlist->name), this);
	mediaContainer = new QWidget(this);
	mediaLayout = new QVBoxLayout(mediaContainer);

	// Initially hide media items
	mediaContainer->setVisible(expanded);
	layout->addWidget(toggleButton);
	layout->addWidget(mediaContainer);
	setLayout(layout);

	// Populate media items
	/* for (size_t i = 0; i < playlist->queue.size(); i++) {
		const MediaFileData *media_file_data = &playlist->queue[i];
		mediaLayout->addWidget(new MediaWidget(media_file_data, this));
	} */
	mediaContainer->setLayout(mediaLayout);

	// Toggle playlist expansion
	connect(toggleButton, &QPushButton::clicked, this, &PlaylistWidget::toggleMediaVisibility);
}

void PlaylistWidget::toggleMediaVisibility()
{
	expanded = !expanded;
	mediaContainer->setVisible(expanded);
}

void PlaylistWidget::update_playlist_data()
{
	for (size_t i = 0; i < playlist->queue.size(); i++) {
		const MediaFileData *media_file_data = &playlist->queue[i];
		media_file_data->media_widget->update_media_file_data();
	}
}

void PlaylistWidget::remove_widget()
{
	QWidget *parent = parent;
	QBoxLayout *layout = qobject_cast<QBoxLayout *>(parent->layout());
	layout->removeWidget(this);
}

#pragma endregion