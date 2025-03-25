#ifndef CUSTOM_PROPERTIES_HPP
#define CUSTOM_PROPERTIES_HPP

#include "../include/qt_classes/custom_properties.hpp"

// CustomProperties::CustomProperties() : QDialog() {}
PlaylistQueueViewer::PlaylistQueueViewer(/* const QString &title,  */ QWidget *parent)
	: QDockWidget("Playlist Media Queue", parent)
{
	QWidget *mainWidget = new QWidget(this);

	bool set_layout = false;

	QVBoxLayout *layout = new QVBoxLayout(mainWidget);

	QLabel *label = new QLabel("Hello from Custom Dock!", mainWidget);

	label->setAlignment(Qt::AlignCenter);

	layout->addWidget(label);

	setVisible(false);
	setFloating(true);
	resize(300, 300);

	mainWidget->setLayout(layout);

	setWidget(mainWidget);
}

PlaylistQueueViewer::~PlaylistQueueViewer() {}

#endif // CUSTOM_PROPERTIES_DIALOG_HPP