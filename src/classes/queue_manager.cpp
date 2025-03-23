#include "../../include/classes/queue_manager.hpp"

QueueManager::QueueManager(MediaFileDataArray *queue, MediaFileDataArray *previous_queue)
{
	p_queue = queue;
	p_previous_queue = previous_queue;
}

QueueManager::~QueueManager()
{
	// p_queue = queue;
	// p_previous_queue = previous_queue;
}