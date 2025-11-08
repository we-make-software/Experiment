#ifndef MinuteMemoryTimeout_H
#define MinuteMemoryTimeout_H
#define TemplateMemoryTimeout MinuteMemoryTimeout

#include "../TemplateMemoryTimeout/.h"

#undef TemplateMemoryTimeout

#define CBuildConnectMinuteMemoryTimeout \
        CBuildConnectApplicationProgrammingInterface(MinuteMemoryTimeout)

#endif