#include "../include/utils/enum-utils.h"

#define X(name) #name,

const char *StartBehavior[] = {ENUM_START_BEHAVIOR_LIST ""};
const char *EndBehavior[] = {ENUM_END_BEHAVIOR_LIST ""};

#undef X