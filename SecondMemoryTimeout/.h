#ifndef SecondMemoryTimeout_H
#define SecondMemoryTimeout_H
#define TemplateMemoryTimeout SecondMemoryTimeout

#include "../TemplateMemoryTimeout/.h"

#undef TemplateMemoryTimeout

#define CBuildConnectSecondMemoryTimeout \
        CBuildConnectApplicationProgrammingInterface(SecondMemoryTimeout)

#endif