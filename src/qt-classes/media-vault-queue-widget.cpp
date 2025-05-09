#pragma region Main

#include "../../include/qt-classes/media-vault-queue-widget.hpp"
#include "../../include/qt-classes/media-widget.hpp"

MediaVaultQueueWidget::MediaVaultQueueWidget(const MediaVaultContext *media_vault_context, QWidget *parent,
					     bool is_main_widget)
	: QWidget(parent),
	  expanded(is_main_widget),
	  is_main_widget(is_main_widget)
{
	this->media_vault_context = media_vault_context;

	QWidget *media_container_root = this;

	// Main layout for the PlaylisQueuetWidget
	layout = new QVBoxLayout(this);

	QBoxLayout *media_container_root_layout = layout;

	if (is_main_widget == true) {
		setWindowTitle(QString::fromStdString(media_vault_context->name + " Queue"));

		// Scroll area setup
		scrollArea = new QScrollArea(this);
		// scrollArea->setSizePolicy();
		scrollArea->setWidgetResizable(true);

		// Container inside scroll area
		contentWidget = new QWidget(this);
		contentLayout = new QVBoxLayout(contentWidget);

		contentWidget->setLayout(contentLayout);

		scrollArea->setWidget(contentWidget);

		layout->addWidget(scrollArea);

		media_container_root = contentWidget;
		media_container_root_layout = contentLayout;
	} else {
		// Create a layout for the toggleButton to make it expand horizontally
		buttonLayout = new QHBoxLayout();

		toggleButton = new QPushButton(QString::fromStdString(media_vault_context->name), this);
		toggleButton->setSizePolicy(QSizePolicy::Minimum,
					    QSizePolicy::Fixed); // Make button expand horizontally

		toggleButton->setStyleSheet("QPushButton {"
					    "border: 1px solid rgb(53, 57, 70);" // Border for the button
					    "border-radius: 5px;"                // Rounded corners
					    "padding: 10px;"                     // Some padding
					    "background-color:rgb(54, 53, 66);"  // Light background
					    "font-size: 14px;"                   // Font size
					    "text-align: center;"                // Text aligned to the left
					    "}"
					    "QPushButton:pressed {"
					    "background-color:rgb(59, 58, 78);" // Darker background when pressed
					    "}");

		buttonLayout->addWidget(toggleButton);
		buttonLayout->setAlignment(Qt::AlignTop);

		layout->addLayout(buttonLayout); // This ensures the button takes up full width
	}

	mediaContainer = new QWidget(media_container_root);
	mediaLayout = new QVBoxLayout(mediaContainer);

	// Shrink the mediaContainer horizontally
	mediaContainer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); // Fixed size for vertical

	// Set the mediaLayout properties to shrink to the content
	mediaLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
	mediaLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter); // Align to top and center

	// Initially hide media items
	mediaContainer->setVisible(expanded);

	// Add the button layout and media container to the main layout
	media_container_root_layout->addWidget(mediaContainer);

	mediaContainer->setLayout(mediaLayout);
	mediaContainer->adjustSize(); // Shrink the container to fit its content

	// Ensure that the container is aligned at the top
	media_container_root_layout->setAlignment(mediaContainer,
						  Qt::AlignTop); // Align mediaContainer to the top when collapsed

	if (is_main_widget == true) {
		resize(600, 500);
		setVisible(false);
		setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint); // Ensure it stays above others
		setAttribute(Qt::WA_ShowWithoutActivating);
	} else {
		// Toggle media_vault expansion
		connect(toggleButton, &QPushButton::clicked, this, &MediaVaultQueueWidget::toggleMediaVisibility);
	}

	setLayout(layout);
}

void MediaVaultQueueWidget::toggleMediaVisibility()
{
	if (mediaContainer == nullptr)
		return;
	expanded = !expanded;

	mediaContainer->setVisible(expanded);
}

void MediaVaultQueueWidget::update_media_vault_name()
{
	if (toggleButton == nullptr)
		return;
	if (is_main_widget == true) {
		setWindowTitle(QString::fromStdString(media_vault_context->name + " Queue"));
	} else {
		toggleButton->setText(QString::fromStdString(media_vault_context->name));
	}
}

void MediaVaultQueueWidget::update_media_vault_data(e_MediaStringifyTYPE media_stringify_type)
{
	MediaVaultQueueWidget::update_media_vault_name();
	for (size_t i = 0; i < media_vault_context->queue.size(); i++) {
		media_vault_context->queue[i]->media_widget->update_media_data(&media_stringify_type);
	}
}

void MediaVaultQueueWidget::remove_widget()
{
	QWidget *parent_widget = parentWidget();
	if (parent_widget) {
		QLayout *layout = parent_widget->layout();
		if (layout) {
			layout->removeWidget(this);
		}
	}
}

MediaWidget *MediaVaultQueueWidget::create_media_widget(MediaContext *media_context,
							e_MediaStringifyTYPE media_stringify_type)
{
	// Create an event loop to ensure synchronous execution
	QEventLoop loop;
	MediaWidget *widget = nullptr;

	// Create the MediaWidget on the main thread
	QMetaObject::invokeMethod(
		this,
		[=, &widget, &loop, &media_stringify_type]() {
			widget = new MediaWidget(media_context, media_stringify_type, this);
			loop.quit(); // Exit the event loop once widget is created
		},
		Qt::QueuedConnection);

	// Block until the widget is created
	loop.exec();

	// Now that the widget is created, add it to the layout
	return widget;
	// Store or handle the widget as necessary
}

void MediaVaultQueueWidget::push_media_widget_front(MediaWidget *mediaWidget)
{
	push_media_widget_at(mediaWidget, 0);
}
void MediaVaultQueueWidget::push_media_widget_back(MediaWidget *mediaWidget)
{
	QWidget *parent_widget = parentWidget();
	if (parent_widget) {
		push_media_widget_at(mediaWidget, parent_widget->children().size());
	}
}

void MediaVaultQueueWidget::push_media_widget_at(MediaWidget *mediaWidget, size_t index)
{
	QEventLoop loop;
	QMetaObject::invokeMethod(
		this,
		[=, &loop, &index]() {
			mediaLayout->insertWidget((int)index, mediaWidget);
			loop.quit(); // Exit the event loop once widget is created
		},
		Qt::QueuedConnection);
	loop.exec();
}

#pragma endregion