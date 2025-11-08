#define TemplateMemoryTimeout SecondMemoryTimeout

#define EventTriggerCode\
    schedule_delayed_work(&DelayedWork, HZ);

#define IndexTickCode\
    (ktime_to_ns(*time) / 1000000000ULL) % 60;

#include "../TemplateMemoryTimeout/.c"