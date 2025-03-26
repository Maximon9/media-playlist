#include "../../include/qt-classes/playlist-widget.hpp"
#include "../../include/qt-classes/playlist-media-widget.hpp"

PlaylistWidget::PlaylistWidget(const PlaylistData *playlist, QWidget *parent) : QWidget(parent), expanded(false)
{
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
	for (size_t i = 0; i < playlist->queue.size(); i++) {
		const MediaFileData *media = &playlist->queue[i];
		mediaLayout->addWidget(new MediaWidget(QString::fromStdString(media->name), this));
	}
	mediaContainer->setLayout(mediaLayout);

	// Toggle playlist expansion
	connect(toggleButton, &QPushButton::clicked, this, &PlaylistWidget::toggleMediaVisibility);
}

void PlaylistWidget::toggleMediaVisibility()
{
	expanded = !expanded;
	mediaContainer->setVisible(expanded);
}