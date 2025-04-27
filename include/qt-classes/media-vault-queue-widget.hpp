#ifndef PLAYLIST_QUEUE_VIEWER_HPP
#define PLAYLIST_QUEUE_VIEWER_HPP

#include <QEventLoop>
#include <QScrollArea>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../include/types/media-vault-source-types.hpp"

typedef struct MediaVaultContext MediaVaultContext;

class MediaVaultQueueWidget : public QWidget {
private:
	QVBoxLayout *layout;
	QPushButton *toggleButton;
	QHBoxLayout *buttonLayout;
	QWidget *mediaContainer;
	QVBoxLayout *mediaLayout;
	e_MediaStringifyTYPE media_stringify_type;
	bool expanded;

#pragma region Main Widget Params
	bool is_main_widget;
	QScrollArea *scrollArea;
	QWidget *contentWidget;
	QVBoxLayout *contentLayout;
#pragma endregion

public:
	const MediaVaultContext *media_vault_context;
	explicit MediaVaultQueueWidget(const MediaVaultContext *media_vault, QWidget *parent = nullptr,
				       bool is_main_widget = false);
	void toggleMediaVisibility();
	void update_media_vault_name();
	void update_media_vault_data(e_MediaStringifyTYPE media_stringify_type = MEDIA_STRINGIFY_TYPE_FILENAME);
	void remove_widget();
	// void create_media_widget(MediaContext *media_context, std::function<void(MediaWidget *)> callback);
	MediaWidget *create_media_widget(MediaContext *media_context,
					 e_MediaStringifyTYPE media_stringify_type = MEDIA_STRINGIFY_TYPE_FILENAME);
	void push_media_widget_front(MediaWidget *mediaWidget);
	void push_media_widget_back(MediaWidget *mediaWidget);
	void push_media_widget_at(MediaWidget *mediaWidget, size_t index);
};
#endif // PLAYLIST_QUEUE_VIEWER_HPP