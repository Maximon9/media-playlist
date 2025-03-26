#ifndef CUSTOM_PROPERTIES_HPP
#define CUSTOM_PROPERTIES_HPP

#include "../../include/qt-classes/custom-properties.hpp"

// CustomProperties::CustomProperties() : QDialog() {}
PlaylistQueueViewer::PlaylistQueueViewer(QWidget *parent = nullptr) : QWidget(parent)
{
	this->parent = parent;

	this->layout = new QVBoxLayout(this);

	this->label = new QLabel("Hello from Custom Dock!", this);

	this->label->setAlignment(Qt::AlignCenter);

	this->layout->addWidget(label);

	setVisible(false);
	// setFloating(true);
	resize(300, 300);

	setLayout(this->layout);
}

PlaylistQueueViewer::~PlaylistQueueViewer()
{
	// delete this->label;
	// delete this->layout;
}

#endif // CUSTOM_PROPERTIES_DIALOG_HPP