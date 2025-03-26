#ifndef CUSTOM_PROPERTIES_HPP
#define CUSTOM_PROPERTIES_HPP

#include "../include/qt_classes/custom_properties.hpp"

// CustomProperties::CustomProperties() : QDialog() {}
PlaylistQueueViewer::PlaylistQueueViewer() : PlaylistQueueViewer(this->parent) {}

PlaylistQueueViewer::PlaylistQueueViewer(QWidget *parent = nullptr) : QDockWidget(parent)
{
	/*
	QWidget *mainWidget = new QWidget(this);

	QVBoxLayout *layout = new QVBoxLayout(mainWidget);

	QLabel *label = new QLabel("Hello from Custom Dock!", mainWidget);
	layout->addWidget(label);

	setVisible(false);
	setFloating(true);
	resize(300, 300);

	if (mainWidget->layout() == nullptr) {
		mainWidget->setLayout(layout);
	}
	setWidget(mainWidget);
	*/
	// this->mainWidget = new QWidget(this);
	// this->parent = parent;

	// this->layout = new QVBoxLayout(this->mainWidget);

	// this->label = new QLabel("Hello from Custom Dock!", this->mainWidget);

	// this->label->setAlignment(Qt::AlignCenter);

	// this->layout->addWidget(label);

	// setVisible(false);
	// setFloating(true);
	// resize(300, 300);

	// this->mainWidget->setLayout(this->layout);
}

PlaylistQueueViewer::~PlaylistQueueViewer()
{
	delete this->label;
	delete this->layout;
	delete this->mainWidget;
}

#endif // CUSTOM_PROPERTIES_DIALOG_HPP