#include "../include/qt-classes/playlist-queue-viewer.hpp"

PlaylistQueueViewer::PlaylistQueueViewer(const PlaylistData *playlist, QWidget *parent = nullptr)
	: PlaylistQueueViewer(playlist, parent)
{
	layout = new QVBoxLayout(this);

	// Scroll area setup
	scrollArea = new QScrollArea(this);
	// scrollArea->setSizePolicy();
	scrollArea->setWidgetResizable(true);

	// Container inside scroll area
	contentWidget = new QWidget();
	contentLayout = new QVBoxLayout(contentWidget);

	contentWidget->setLayout(contentLayout);

	contentLayout->setAlignment(Qt::AlignTop);

	scrollArea->setWidget(contentWidget);

	layout->addWidget(scrollArea);
	setLayout(layout);
}