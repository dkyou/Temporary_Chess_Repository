#include <stdio.h>
#include "app/controller.h"

int main(void){
    AppController* c = app_controller_create();
    app_controller_new_game(c);
    AppDrawModel m;
    app_controller_get_draw_model(c, &m);
    printf("CLI stub. Type four ints (fx fy tx ty) to move, or EOF to quit.\n");
    for(;;){
        int fx, fy, tx, ty;
        if(scanf("%d %d %d %d", &fx,&fy,&tx,&ty)!=4) break;
        app_controller_click_cell(c, fx, fy);
        app_controller_click_cell(c, tx, ty);
        app_controller_get_draw_model(c, &m);
        printf("Moved. Side to move=%d\n", m.side_to_move);
    }
    app_controller_destroy(c);
    return 0;
}
