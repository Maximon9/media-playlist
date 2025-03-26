#ifndef CUSTOM_PROPERTIES_HPP
#define CUSTOM_PROPERTIES_HPP

#include "../../include/qt-classes/playlist-queue-viewer.hpp"

// CustomProperties::CustomProperties() : QDialog() {}
PlaylistQueueViewer::PlaylistQueueViewer(QWidget *parent = nullptr) : QWidget(parent)
{
	this->playlist_datas = {};

	this->parent = parent;

	this->layout = new QVBoxLayout(this);

	this->layout->addWidget(label);

	resize(300, 300);

	setLayout(this->layout);
}

PlaylistQueueViewer::~PlaylistQueueViewer() {}

#endif // CUSTOM_PROPERTIES_DIALOG_HPP