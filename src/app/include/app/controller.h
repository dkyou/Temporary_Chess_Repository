#pragma once
#include <stdint.h>
#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif
#define BOARD_WIDTH 900
#define BOARD_HEIGHT 1000
typedef struct AppController AppController;

/* 一步走法（格点） */
typedef struct { int fx, fy, tx, ty; } AppMove;

/* 渲染模型（UI需要的最小数据） */
typedef struct {
    int rows, cols;                /* 10x9 */
    int side_to_move;              /* enum Player -> int */
    int selected_x, selected_y;    /* -1 表示无选中（格点坐标：col=x, row=y） */
    int board[10][9];              /* enum PieceType -> int */

    /* 选中后可走提示 */
    int  legal_count;
    AppMove legal[128];

    /* 提示状态 */
    int in_check_red;              /* 红方被将：1 是 / 0 否 */
    int in_check_black;            /* 黑方被将：1 是 / 0 否 */
    int face_to_face;              /* 将帅对脸：1 是 / 0 否 */
} AppDrawModel;

AppController* app_controller_create(void);
void           app_controller_destroy(AppController*);

/* 拉取一帧可绘制数据 */
void app_controller_get_draw_model(const AppController*, AppDrawModel* out);

/* UI 把“格点坐标”的点击喂给控制器（像素→格点由端口计算） */
void app_controller_click_cell(AppController*, int gx, int gy);

/* 命令式动作 */
void app_controller_new_game(AppController*);
int  app_controller_save(AppController*, const char* path);
int  app_controller_load(AppController*, const char* path);
int  app_controller_undo(AppController*);   /* 0 成功，-1 失败 */

/* 棋子标签/颜色（UI 渲染用） */
typedef enum { APP_COLOR_NONE=0, APP_COLOR_RED=1, APP_COLOR_BLACK=2 } AppPieceColor;
const char*    app_piece_label(int piece_code);
AppPieceColor  app_piece_color(int piece_code);

/* 新增：返回 PNG 文件名（不含扩展名），例如 r_general / b_horse */
const char*    app_piece_png_id(int piece_code);
/* 是否为将/帥 */
int app_piece_is_general(int piece_code);
void app_controll_ScreenToBoard_from_AppController(struct AppController* c, int x, int y, int* row, int* col);
void app_controll_boardToScreen_from_AppController(struct AppController* c, int row, int col, int* x, int* y);
void app_controll_ScreenToBoard(const AppDrawModel* model, int x, int y, int* row, int* col);
void app_controll_boardToScreen(const AppDrawModel* model, int row, int col, int* x, int* y);
#ifdef __cplusplus
}
#endif



#if 0
#pragma once
#include <stdint.h>
#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AppController AppController;

/* 一步走法（格点） */
typedef struct { int fx, fy, tx, ty; } AppMove;

/* 渲染模型（UI需要的最小数据） */
typedef struct {
    int rows, cols;                /* 10x9 */
    int side_to_move;              /* enum Player -> int */
    int selected_x, selected_y;    /* -1 表示无选中（格点坐标：col=x, row=y） */
    int board[10][9];              /* enum PieceType -> int */
    /* 选中后可走提示 */
    int  legal_count;
    AppMove legal[128];
} AppDrawModel;

AppController* app_controller_create(void);
void           app_controller_destroy(AppController*);

/* 拉取一帧可绘制数据 */
void app_controller_get_draw_model(const AppController*, AppDrawModel* out);

/* UI 把“格点坐标”的点击喂给控制器（像素→格点由端口计算） */
void app_controller_click_cell(AppController*, int gx, int gy);

/* 命令式动作 */
void app_controller_new_game(AppController*);
int  app_controller_save(AppController*, const char* path);
int  app_controller_load(AppController*, const char* path);
int  app_controller_undo(AppController*);   /* 0 成功，-1 失败 */

/* 供 UI 获取文本/颜色提示（把核心枚举隐藏在控制器里） */
typedef enum { APP_COLOR_NONE=0, APP_COLOR_RED=1, APP_COLOR_BLACK=2 } AppPieceColor;
const char*    app_piece_label(int piece_code);
AppPieceColor  app_piece_color(int piece_code);

#ifdef __cplusplus
}
#endif
#endif