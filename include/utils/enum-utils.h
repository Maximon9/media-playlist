#ifndef ENUM_UTILS_H
#define ENUM_UTILS_H

// Define the StartBehavior enum values
#define ENUM_START_BEHAVIOR_LIST     \
	SX(RESTART_ENTIRE_PLAYLIST)  \
	SX(RESTART_AT_CURRENT_INDEX) \
	SX(KEEP_SAME_BEHAVIOR)       \
	SX(UNPAUSE)                  \
	SX(PAUSE)

// Define the EndBehavior enum values
#define ENUM_END_BEHAVIOR_LIST \
	EX(STOP)               \
	EX(LOOP)               \
	EX(LOOP_AT_INDEX)      \
	EX(LOOP_AT_END)

// Define the LoopEndBehavior enum values
#define ENUM_LOOP_END_BEHAVIOR_LIST \
	LX(STOP)                    \
	LX(RESTART)

// Define the enum values for StartBehavior, EndBehavior, and LoopEndBehavior
#define SX(name) name,
typedef enum { ENUM_START_BEHAVIOR_LIST } StartBehavior;
#undef SX

#define EX(name) name,
typedef enum { ENUM_END_BEHAVIOR_LIST } EndBehavior;
#undef EX

#define LX(name) name,
typedef enum { ENUM_LOOP_END_BEHAVIOR_LIST } LoopEndBehavior;
#undef LX

// Create lookup tables using macros for the string representations
#define SX(name) #name,
const char *StartBehaviorStrings[];
#undef SX

#define EX(name) #name,
const char *EndBehaviorStrings[];
#undef EX

#define LX(name) #name,
const char *LoopEndBehaviorStrings[];
#undef LX

#endif // ENUM_UTILS_H