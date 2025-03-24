#ifndef CUSTOM_PROPERTIES_HPP
#define CUSTOM_PROPERTIES_HPP

#include "../include/qt_classes/custom_properties.hpp"
#include "Qt"

// CustomProperties::CustomProperties() : QDialog() {}
PlaylistQueueViewer::PlaylistQueueViewer(/* const QString &title,  */ QWidget *parent)
	: QDockWidget("Playlist Media Queue", parent)
{
	QWidget *mainWidget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(mainWidget);

	QLabel *label = new QLabel("Hello from Custom Dock!", mainWidget);
	layout->addWidget(label);

	setVisible(false);
	setFloating(true);
	resize(300, 300);

	mainWidget->setLayout(layout);
	setWidget(mainWidget);

	setLayout(layout);
}
PlaylistQueueViewer::~PlaylistQueueViewer() {}

#endif // CUSTOM_PROPERTIES_DIALOG_HPP