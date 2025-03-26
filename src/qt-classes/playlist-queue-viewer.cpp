#ifndef CUSTOM_PROPERTIES_HPP
#define CUSTOM_PROPERTIES_HPP

#include "../../include/qt-classes/playlist-queue-viewer.hpp"

// CustomProperties::CustomProperties() : QDialog() {}
PlaylistQueueViewer::PlaylistQueueViewer(QWidget *parent = nullptr) : QWidget(parent)
{
	this->playlist_datas = {};

	this->layout = new QVBoxLayout(this);

	resize(300, 300);

	setLayout(this->layout);
}

#endif // CUSTOM_PROPERTIES_DIALOG_HPP