#include <QWidget>
#include <QSlider>
#include <QSpinBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <obs-module.h>

class CustomProperties : public QDialog {
	// Q_OBJECT
	// private:
	// QSpinBox queue_list_size;
	// MediaFileDataArray all_media;
	// obs_source_t *source;
	// obs_source_t *media_source;
	// bool shuffle_queue;
	// e_StartBehavior playlist_start_behavior;
	// e_EndBehavior playlist_end_behavior;
	// int loop_index;
	// bool infinite;
	// int loop_count;
	// e_LoopEndBehavior loop_end_behavior;
	// int song_history_limit;
	// bool debug;

public:
public:
	explicit CustomProperties(obs_source_t *source, QWidget *parent = nullptr);

	// Destructor: This is called when an object is destroyed.
	// ~CustomProperties();
};