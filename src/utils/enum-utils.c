#include "../include/utils/enum-utils.h"

#define X(name) #name,

const char *StartBehaviorName[] = {ENUM_START_BEHAVIOR_LIST};
const char *EndBehaviorName[] = {ENUM_END_BEHAVIOR_LIST};

#undef X