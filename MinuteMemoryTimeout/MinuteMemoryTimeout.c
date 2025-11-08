#define TemplateMemoryTimeout MinuteMemoryTimeout

#define EventTriggerCode\
    struct timespec64 ts;\
    ktime_get_real_ts64(&ts);\
    schedule_delayed_work(&DelayedWork,(60-ts.tv_sec % 60)*HZ);
    
#define IndexTickCode\
     (ktime_to_ns(*time)/(60000000000ULL))%60;

#include "../TemplateMemoryTimeout/.c"