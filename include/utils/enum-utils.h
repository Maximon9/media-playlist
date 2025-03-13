#ifndef ENUM_UTILS_H
#define ENUM_UTILS_H

#define ENUM_START_BEHAVIOR_LIST \
	X(RESTART)               \
	X(UNPAUSE)

#define ENUM_END_BEHAVIOR_LIST \
	X(STOP)                \
	X(LOOP)                \
	X(LOOP_SPECIFIC_MEDIA) \
	X(LOOP_LAST_MEDIA)

#define X(name) name,

enum { ENUM_START_BEHAVIOR_LIST } StartBehavior;
enum { ENUM_END_BEHAVIOR_LIST } EndBehavior;

#undef X

// Create a lookup table using macros
#define X(name) #name,

const char *StartBehaviorName[];
const char *EndBehaviorName[];

#undef X

#endif // ENUM_UTILS_H