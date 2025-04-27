#pragma region Main

#include "../../include/qt-classes/multi-media-vault-queue-viewer.hpp"
#include "../../include/qt-classes/media-vault-queue-widget.hpp"

MultiMediaVaultQueueViewer::MultiMediaVaultQueueViewer(QWidget *parent) : QWidget(parent)
{
	this->media_vault_datas = {};
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

	resize(600, 600);

	setLayout(layout);
}

void MultiMediaVaultQueueViewer::addMediaVaultWidget(MediaVaultQueueWidget *media_vault_widget)
{
	multi_media_vault_queue_viewer->contentLayout->addWidget(media_vault_widget);
}

void MultiMediaVaultQueueViewer::updateMediaVaults()
{
	// Add new media_vaults
	for (size_t i = 0; i < this->media_vault_datas.size(); i++) {
		const MediaVaultData *media_vault_data = this->media_vault_datas[i];
		if (media_vault_data == nullptr) {
			continue;
		}
		media_vault_data->media_vault_widget->update_media_vault_data();
	}
}

MultiMediaVaultQueueViewer *multi_media_vault_queue_viewer = nullptr;

#pragma endregion