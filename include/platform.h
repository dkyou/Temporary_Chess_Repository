#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t pal_now_ms(void);
void     pal_sleep_ms(uint32_t ms);

int  pal_file_size(const char* path, size_t* out_size);
int  pal_read_file(const char* path, void* buf, size_t bufsize, size_t* out_read);

void pal_log_info(const char* fmt, ...);
void pal_log_warn(const char* fmt, ...);
void pal_log_error(const char* fmt, ...);

typedef struct PalImage PalImage;
PalImage* pal_image_load(const char* path);
void      pal_image_free(PalImage*);
int       pal_image_size(PalImage*, int* w, int* h);

typedef enum { 
    EV_NONE=0, EV_POINTER_DOWN, EV_POINTER_MOVE, EV_POINTER_UP, 
    EV_KEY_DOWN, EV_KEY_UP, EV_TICK 
} pal_event_type_t;

typedef struct { pal_event_type_t type; int32_t x,y; int32_t key; uint32_t mods; } pal_event_t;

typedef void (*pal_event_cb)(const pal_event_t* ev, void* ud);
int  pal_event_subscribe(pal_event_cb cb, void* ud);
int  pal_event_unsubscribe(pal_event_cb cb, void* ud);
void pal_emit(const pal_event_t* ev);

#ifdef __cplusplus
}
#endif
