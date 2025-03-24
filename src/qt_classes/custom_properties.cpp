#ifndef CUSTOM_PROPERTIES_HPP
#define CUSTOM_PROPERTIES_HPP

#include "../include/qt_classes/custom_properties.hpp"
#include "Qt"

// CustomProperties::CustomProperties() : QDialog() {}
CustomProperties::CustomProperties(obs_source_t *source, QWidget *parent) : QDialog(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	QLabel *sliderLabel = new QLabel("Slider Value:", this);
	QSlider *slider = new QSlider(Qt::Horizontal, this);
	QSpinBox *sliderSpinBox = new QSpinBox(this);

	slider->setRange(0, 100); // Adjust as needed
	sliderSpinBox->setRange(0, 100);

	// Sync slider and spinbox
	connect(slider, &QSlider::valueChanged, sliderSpinBox, &QSpinBox::setValue);
	connect(sliderSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);

	// Up/Down Arrows + Editable Number Input
	QLabel *numberLabel = new QLabel("Number Input:", this);
	QSpinBox *numberSpinBox = new QSpinBox(this);
	numberSpinBox->setRange(-100, 100); // Adjust range as needed

	layout->addWidget(sliderLabel);
	layout->addWidget(slider);
	layout->addWidget(sliderSpinBox);
	layout->addWidget(numberLabel);
	layout->addWidget(numberSpinBox);
	setLayout(layout);
}
// CustomProperties::~CustomProperties() {}

#endif // CUSTOM_PROPERTIES_DIALOG_HPP