#ifdef __linux__
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#endif

#include <platform.h>

uint64_t pal_now_ms(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timeval tv; gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec*1000ULL + (uint64_t)tv.tv_usec/1000ULL;
#endif
}

void pal_sleep_ms(uint32_t ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

static void vlog(FILE* fp, const char* tag, const char* fmt, va_list ap) {
    fprintf(fp, "[%s] ", tag); vfprintf(fp, fmt, ap); fputc('\n', fp);
}
void pal_log_info(const char* fmt, ...)  { va_list ap; va_start(ap, fmt); vlog(stdout,"INFO",fmt,ap); va_end(ap); }
void pal_log_warn(const char* fmt, ...)  { va_list ap; va_start(ap, fmt); vlog(stdout,"WARN",fmt,ap); va_end(ap); }
void pal_log_error(const char* fmt, ...) { va_list ap; va_start(ap, fmt); vlog(stderr,"ERR ",fmt,ap); va_end(ap); }

int pal_file_size(const char* path, size_t* out_size){
    FILE* f = fopen(path,"rb"); if(!f) return -1;
    fseek(f,0,SEEK_END);
    long n = ftell(f);
    fclose(f);
    if(n < 0) {
        return -1;
    }
    *out_size=(size_t)n;
    return 0;
}

int pal_read_file(const char* path, void* buf, size_t bufsize, size_t* out_read){
    FILE* f = fopen(path,"rb"); if(!f) return -1;
    size_t n = fread(buf,1,bufsize,f);
    fclose(f);
    if (out_read) { *out_read = n; }
    return (n>0)?0:-1;
}

/* Images stubs unless a port defines PAL_IMAGE_EXTERNAL */
#ifndef PAL_IMAGE_EXTERNAL
struct PalImage { int dummy; };
PalImage* pal_image_load(const char* path){ (void)path; return NULL; }
void      pal_image_free(PalImage* p){ (void)p; }
int       pal_image_size(PalImage* p,int* w,int* h){ (void)p; if(w)*w=0; if(h)*h=0; return -1; }
#endif
