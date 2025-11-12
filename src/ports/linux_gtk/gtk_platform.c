#include "platform.h"
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>

struct PalImage { GdkPixbuf* px; };

PalImage* pal_image_load(const char* path){
    GError* err = NULL;
    GdkPixbuf* p = gdk_pixbuf_new_from_file(path, &err);
    if(!p){
        if(err){ fprintf(stderr, "image load failed: %s\n", err->message); g_error_free(err); }
        return NULL;
    }
    PalImage* img = g_new0(PalImage,1);
    img->px = p;
    return img;
}

void pal_image_free(PalImage* img){
    if(!img) return;
    if(img->px) g_object_unref(img->px);
    g_free(img);
}

int pal_image_size(PalImage* img, int* w, int* h){
    if(!img) return -1;
    if(w) *w = gdk_pixbuf_get_width(img->px);
    if(h) *h = gdk_pixbuf_get_height(img->px);
    return 0;
}
