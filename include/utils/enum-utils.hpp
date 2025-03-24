#ifndef ENUM_UTILS_H
#define ENUM_UTILS_H

// Define the StartBehavior enum values
#define ENUM_START_BEHAVIOR_LIST                   \
	X(START_BEHAVIOR_RESTART_ENTIRE_PLAYLIST)  \
	X(START_BEHAVIOR_RESTART_AT_CURRENT_INDEX) \
	X(START_BEHAVIOR_UNPAUSE)                  \
	X(START_BEHAVIOR_PAUSE)

// Define the EndBehavior enum values
#define ENUM_END_BEHAVIOR_LIST        \
	X(END_BEHAVIOR_STOP)          \
	X(END_BEHAVIOR_LOOP)          \
	X(END_BEHAVIOR_LOOP_AT_INDEX) \
	X(END_BEHAVIOR_LOOP_AT_END)

#define ENUM_LOOP_END_BEHAVIOR_LIST \
	X(LOOP_END_BEHAVIOR_STOP)   \
	X(LOOP_END_BEHAVIOR_RESTART)

#define ENUM_MEDIA_STRINGIFY_TYPE        \
	X(MEDIA_STRINGIFY_TYPE_PATH)     \
	X(MEDIA_STRINGIFY_TYPE_FILENAME) \
	X(MEDIA_STRINGIFY_TYPE_NAME)

// Define the enum values for StartBehavior and EndBehavior
#define X(name) name,
typedef enum { ENUM_START_BEHAVIOR_LIST } e_StartBehavior;
typedef enum { ENUM_END_BEHAVIOR_LIST } e_EndBehavior;
typedef enum { ENUM_LOOP_END_BEHAVIOR_LIST } e_LoopEndBehavior;
typedef enum { ENUM_MEDIA_STRINGIFY_TYPE } e_MediaStringifyTYPE;
#undef X

// Create lookup tables using macros for the string representations
#define X(name) #name,
const char *StartBehavior[];
const char *EndBehavior[];
const char *LoopEndBehavior[];
const char *MediaStringifyTYPE[];
#undef X

#endif // ENUM_UTILS_H