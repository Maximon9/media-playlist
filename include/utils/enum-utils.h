#ifndef ENUM_UTILS_H
#define ENUM_UTILS_H

// Define the StartBehavior enum values
#define ENUM_START_BEHAVIOR_LIST \
	X(RESTART)               \
	X(UNPAUSE)               \
	X(PAUSE)

// Define the EndBehavior enum values
#define ENUM_END_BEHAVIOR_LIST \
	X(STOP)                \
	X(LOOP)                \
	X(LOOP_AT_INDEX)

// Define the enum values for StartBehavior and EndBehavior
#define X(name) name,
enum StartBehavior { ENUM_START_BEHAVIOR_LIST };
enum EndBehavior { ENUM_END_BEHAVIOR_LIST };
#undef X

// Create lookup tables using macros for the string representations
#define X(name) #name,
const char *StartBehavior[];
const char *EndBehavior[];
#undef X

#endif // ENUM_UTILS_H