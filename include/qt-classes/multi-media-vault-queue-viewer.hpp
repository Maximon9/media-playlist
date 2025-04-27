#ifndef MULTI_PLAYLIST_QUEUE_VIEWER_HPP
#define MULTI_PLAYLIST_QUEUE_VIEWER_HPP

#include <obs-module.h>
#include <plugin-support.h>
#include <deque>
#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include "../include/types/media-vault-source-types.hpp"

typedef std::deque<MediaVaultData *> PlaylistWidgetDatas;

class MultiMediaVaultQueueViewer : public QWidget {
private:
	QVBoxLayout *layout;
	QScrollArea *scrollArea;
	QWidget *contentWidget;
	QVBoxLayout *contentLayout;

public:
	PlaylistWidgetDatas media_vault_datas;
	explicit MultiMediaVaultQueueViewer(QWidget *parent = nullptr);
	void addMediaVaultWidget(MediaVaultQueueWidget *param_media_vault_widget);
	void updateMediaVaults();
};

extern MultiMediaVaultQueueViewer *multi_media_vault_queue_viewer;

#endif // MULTI_PLAYLIST_QUEUE_VIEWER_HPP