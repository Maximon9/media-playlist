#include "../include/utils/enum-utils.hpp"

#define X(name) #name,

const char *StartBehavior[] = {ENUM_START_BEHAVIOR_LIST ""};
const char *EndBehavior[] = {ENUM_END_BEHAVIOR_LIST ""};
const char *LoopEndBehavior[] = {ENUM_LOOP_END_BEHAVIOR_LIST ""};
const char *MediaStringifyTYPE[] = {ENUM_MEDIA_STRINGIFY_TYPE ""};

#undef X