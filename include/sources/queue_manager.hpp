#include "../utils/utils.hpp"

class QueueManager {
private:
	MediaFileDataArray *p_queue;
	MediaFileDataArray *p_previous_queue;

public:
	QueueManager(MediaFileDataArray *queue, MediaFileDataArray *previous_queue);

	// Destructor: This is called when an object is destroyed.
	~QueueManager();
};