#include <platform.h>
#define MAX_SUBS 8
static struct { pal_event_cb cb; void* ud; } subs[MAX_SUBS];

int pal_event_subscribe(pal_event_cb cb, void* ud){
    for(int i=0;i<MAX_SUBS;i++) if(!subs[i].cb){ subs[i].cb=cb; subs[i].ud=ud; return 0; }
    return -1;
}
int pal_event_unsubscribe(pal_event_cb cb, void* ud){
    for(int i=0;i<MAX_SUBS;i++) if(subs[i].cb==cb && subs[i].ud==ud){ subs[i].cb=0; subs[i].ud=0; return 0; }
    return -1;
}
void pal_emit(const pal_event_t* ev){
    for(int i=0;i<MAX_SUBS;i++) if(subs[i].cb) subs[i].cb(ev, subs[i].ud);
}
