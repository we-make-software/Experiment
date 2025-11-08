#include "../System/.h"
#include <linux/slab.h>  
struct TemplateMemoryTimeout{
    struct list_head timer,binding;   
    struct work_struct work; 
    void(*function)(struct TemplateMemoryTimeout*); 
    u8 tick;
    struct mutex lock;
};
    HBuildStart(TemplateTemplateMemoryTimeout)
        HBuildSignature(void,Disable,(struct TemplateMemoryTimeout*root))
        HBuildSignature(void,Tick,(struct TemplateMemoryTimeout*root,u8*tick))
        HBuildSignature(void,Function,(struct TemplateMemoryTimeout*root,void(*function)(struct TemplateMemoryTimeout*)))
        HBuildSignature(void,Unbinding,(struct TemplateMemoryTimeout*root,struct TemplateMemoryTimeout*bind))
        HBuildSignature(void,UnbindingAll,(struct TemplateMemoryTimeout*root))
        HBuildSignature(void,Binding,(struct TemplateMemoryTimeout*root,struct TemplateMemoryTimeout*bind))
        HBuildSignature(void,BindingCloning,(struct TemplateMemoryTimeout*root,struct TemplateMemoryTimeout*bind))
        HBuildSignature(bool,New,(struct TemplateMemoryTimeout*root,u8*minute,void(*function)(struct TemplateMemoryTimeout*)))
    HBuildEnd
