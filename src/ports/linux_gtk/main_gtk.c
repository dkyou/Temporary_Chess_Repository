#include <gtk/gtk.h>
#include <platform.h>
// #include <chess.h> //使用控制器不直接接触chess.h
#include <gtk/ui.h>

int main(int argc, char** argv){
    pal_log_info("Starting GTK chess (controller-driven)…");
    return gtk_chess_main(argc, argv);
}
