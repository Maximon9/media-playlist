#ifndef ENUM_UTILS_H
#define ENUM_UTILS_H

// Define the StartBehavior enum values
#define ENUM_START_BEHAVIOR_LIST   \
	X(RESTART_ENTIRE_PLAYLIST) \
	X(RESTART_AT_CURRENT_INEX) \
	X(KEEP_SAME_BEHAVIOR)      \
	X(UNPAUSE)                 \
	X(PAUSE)

// Define the EndBehavior enum values
#define ENUM_END_BEHAVIOR_LIST \
	X(STOP)                \
	X(LOOP)                \
	X(LOOP_AT_INDEX)       \
	X(LOOP_AT_END)

#define ENUM_LOOP_END_BEHAVIOR_LIST \
	X(STOP)                     \
	X(RESTART)

// Define the enum values for StartBehavior and EndBehavior
#define X(name) name,
enum StartBehavior { ENUM_START_BEHAVIOR_LIST };
enum EndBehavior { ENUM_END_BEHAVIOR_LIST };
enum LoopEndBehavior { ENUM_LOOP_END_BEHAVIOR_LIST };
#undef X

// Create lookup tables using macros for the string representations
#define X(name) #name,
const char *StartBehavior[];
const char *EndBehavior[];
const char *LoopEndBehavior[];
#undef X

#endif // ENUM_UTILS_H