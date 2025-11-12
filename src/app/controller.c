#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chess.h>
#include "app/controller.h"

struct AppController {
    struct Game game;  /* 直接持有 Game */
    int selected_x, selected_y;
    double current_width, current_height; /* 新增：返回当前屏幕尺寸，提供给 ScreenToBoard 等接口使用 */
    struct {
        int n, cap[1024];
        struct Move mv[1024];
    } hist;
};

AppController* app_controller_create(void){
    AppController* c = (AppController*)calloc(1, sizeof(*c));
    c->selected_x = c->selected_y = -1;
    Game_initialize(&c->game);
    return c;
}

void app_controller_destroy(AppController* c){
    if(!c) return;
    free(c);
}

void app_controller_new_game(AppController* c){
    Game_initialize(&c->game);
    c->selected_x = c->selected_y = -1;
    c->hist.n = 0;
}

int app_controller_save(AppController* c, const char* path){
    return Game_saveGame(&c->game, path) ? 0 : -1;
}

int app_controller_load(AppController* c, const char* path){
    int ok = Game_loadGame(&c->game, path) ? 0 : -1;
    c->selected_x = c->selected_y = -1;
    c->hist.n = 0;
    return ok;
}

static int piece_at(struct AppController* c, int row, int col){
    return (int)Chessboard_getPieceAt(c->game.chessboard, row, col);
}

/* 找到红将/黑将位置（返回 1=找到）*/
static int find_generals(struct AppController* c, int* rx, int* ry, int* bx, int* by){
    int fr=0, fb=0;
    for(int r=0;r<10;r++){
        for(int x=0;x<9;x++){
            int p = piece_at(c,r,x);
            if(p==R_GENERAL){ *ry=r; *rx=x; fr=1; }
            if(p==B_GENERAL){ *by=r; *bx=x; fb=1; }
        }
    }
    return fr && fb;
}

/* 将帅对脸：同列且中间无子 */
static int is_face_to_face(struct AppController* c){
    int rx=0,ry=0,bx=0,by=0;
    if(!find_generals(c,&rx,&ry,&bx,&by)) return 0;
    if(rx!=bx) return 0;
    int x = rx;
    int y1 = ry < by ? ry+1 : by+1;
    int y2 = ry < by ? by-1 : ry-1;
    for(int y=y1; y<=y2; ++y){
        if(piece_at(c,y,x)!=EMPTY) return 0; /* 中间有子 -> 不是对脸 */
    }
    return 1;
}

/* side 被将？（对方是否有合法步能吃到该 side 的将/帅） */
static int is_in_check(struct AppController* c, enum Player side){
    int gx=0,gy=0,bx=0,by=0;
    if(!find_generals(c, &gx,&gy, &bx,&by)) return 0;
    int tgtx = (side==RED)? gx: bx;
    int tgty = (side==RED)? gy: by;

    /* 遍历对方所有棋子，看能否合法走到将/帅 */
    for(int r=0;r<10;r++){
        for(int x=0;x<9;x++){
            int p = piece_at(c,r,x);
            if(p==EMPTY) continue;
            /* 归属判断：红方棋子在 R_*，黑方棋子在 B_* */
            int is_red = (p==R_GENERAL||p==R_ADVISOR||p==R_ELEPHANT||p==R_HORSE||p==R_CHARIOT||p==R_CANNON||p==R_SOLDIER);
            enum Player owner = is_red? RED: BLACK;
            if(owner == side) continue; /* 只看对方 */
            struct Move mv = (struct Move){0};
            mv.from.row=r; mv.from.col=x;
            mv.to.row=tgty; mv.to.col=tgtx;
            mv.captured = EMPTY;
            if(Chessboard_isValidMoveQuiet(c->game.chessboard, mv, owner)){//解决开局大量打印
                return 1;
            }
        }
    }
    return 0;
}

void app_controller_get_draw_model(const AppController* c0, AppDrawModel* out){
    AppController* c = (AppController*)c0;
    memset(out, 0, sizeof(*out));
    out->rows = 10; out->cols = 9;
    out->selected_x = c->selected_x;
    out->selected_y = c->selected_y;
    out->side_to_move = (int)Game_getCurrentPlayer(&c->game);

    for(int r=0;r<10;r++)
        for(int x=0;x<9;x++)
            out->board[r][x] = (int)Chessboard_getPieceAt(c->game.chessboard, r, x);

    /* 提示状态 */
    out->face_to_face = is_face_to_face(c);
    out->in_check_red   = is_in_check(c, RED);
    out->in_check_black = is_in_check(c, BLACK);

    /* 若有选中，枚举可走步用于 UI 高亮 */
    if(c->selected_x >= 0){
        int sx = c->selected_x, sy = c->selected_y;
        for(int ty=0; ty<10; ++ty){
            for(int tx=0; tx<9; ++tx){
                if(tx==sx && ty==sy) continue;
                struct Move mv = {0};
                mv.from.row = sy; mv.from.col = sx;
                mv.to.row   = ty; mv.to.col   = tx;
                mv.captured = EMPTY;
                enum Player side = Game_getCurrentPlayer(&c->game);
                if(Chessboard_isValidMoveQuiet(c->game.chessboard, mv, side)){//解决移动时候大量打印
                    if(out->legal_count < (int)(sizeof(out->legal)/sizeof(out->legal[0]))){
                        out->legal[out->legal_count++] = (AppMove){sx,sy,tx,ty};
                    }
                }
            }
        }
    }
}

int app_controller_undo(AppController* c){
    if(!c) return -1;
    if(c->game.chessboard == NULL) return -1;
    if (c->game.history.count <= 0) return -1;

    int lastIndex = c->game.history.count - 1;
    if (lastIndex < 0 || lastIndex >= c->game.history.capacity) {
        c->game.history.count = 0; /* 紧急修复 */
        return -1;
    }

    Chessboard_undoMove(c->game.chessboard, &c->game.history);
    Game_switchPlayer(&c->game);
    c->selected_x = c->selected_y = -1;
    return 0;
}

/* ===== 棋子标记/颜色，方便 UI 画面 ===== */
const char* app_piece_label(int p){
    switch(p){
        case R_GENERAL: return "帥";
        case R_ADVISOR: return "仕";
        case R_ELEPHANT:return "相";
        case R_HORSE:   return "馬";
        case R_CHARIOT: return "車";
        case R_CANNON:  return "炮";
        case R_SOLDIER: return "兵";
        case B_GENERAL: return "將";
        case B_ADVISOR: return "士";
        case B_ELEPHANT:return "象";
        case B_HORSE:   return "馬";
        case B_CHARIOT: return "車";
        case B_CANNON:  return "砲";
        case B_SOLDIER: return "卒";
        default: return "";
    }
}

AppPieceColor app_piece_color(int p){
    switch(p){
        case R_GENERAL: case R_ADVISOR: case R_ELEPHANT: case R_HORSE:
        case R_CHARIOT: case R_CANNON:  case R_SOLDIER:
            return APP_COLOR_RED;
        case B_GENERAL: case B_ADVISOR: case B_ELEPHANT: case B_HORSE:
        case B_CHARIOT: case B_CANNON:  case B_SOLDIER:
            return APP_COLOR_BLACK;
        default: return APP_COLOR_NONE;
    }
}
/* ===== 基名归一：不同颜色枚举 → 同一基名 ===== */
static const char* piece_base_name(int code){
  switch(code){
      case R_GENERAL: case B_GENERAL:  return "general";   // 将/帥
      case R_ADVISOR: case B_ADVISOR:  return "advisor";   // 士/仕
      case R_ELEPHANT: case B_ELEPHANT:return "elephant";  // 象/相
      case R_HORSE:    case B_HORSE:   return "horse";     // 马/馬
      case R_CHARIOT:  case B_CHARIOT: return "chariot";   // 车/車
      case R_CANNON:   case B_CANNON:  return "cannon";    // 炮/砲
      case R_SOLDIER:  case B_SOLDIER: return "soldier";   // 兵/卒
      default: return NULL;
  }
}

/* 对外：返回 PNG 名称（不含扩展名），例如 r_general / b_horse
 注意：返回静态缓冲区，调用后请立刻使用（不要长期保存指针）。 */
const char* app_piece_png_id(int piece_code){
  static char buf[32];
  const char* base = piece_base_name(piece_code);
  if(!base) return NULL;
  AppPieceColor color = app_piece_color(piece_code);
  const char* prefix = (color == APP_COLOR_RED) ? "r" :
                       (color == APP_COLOR_BLACK) ? "b" : NULL;
  if(!prefix) return NULL;
  snprintf(buf, sizeof(buf), "%s_%s", prefix, base);
  return buf;
}
int app_piece_is_general(int p){
  switch(p){
      case R_GENERAL: case B_GENERAL: return 1;
      default: return 0;
  }
}
//新增
void app_controller_click_cell(AppController* c, int gx, int gy){
  if(c->selected_x < 0){
      c->selected_x = gx; c->selected_y = gy;
      return;
  }
  /* 第二次点击：尝试走子 */
  struct Move mv = {0};
  mv.from.row = c->selected_y; mv.from.col = c->selected_x;
  mv.to.row   = gy;             mv.to.col   = gx;
  mv.captured = EMPTY;

  enum Player side = Game_getCurrentPlayer(&c->game);
  if(Chessboard_isValidMove(c->game.chessboard, mv, side)) {
      int captured = piece_at(c, gy, gx); /* 落子前读取被吃子（若你以后要做内置撤销可用到） */
      if(Chessboard_makeMove(c->game.chessboard, mv, &c->game.history)){
          Game_switchPlayer(&c->game);
          if(c->hist.n < (int)(sizeof(c->hist.mv)/sizeof(c->hist.mv[0]))){
              c->hist.mv[c->hist.n]  = mv;
              c->hist.cap[c->hist.n] = captured;
              c->hist.n++;
          }
      }
  }
  c->selected_x = c->selected_y = -1;
}

/* 重点 ----------------------这里需要优化--------------------------------
----------------------------------------------------------------------- */
// 将屏幕坐标(x,y)转换为chessboard[10][9]棋盘坐标(row,col)
void app_controll_ScreenToBoard_from_AppController(struct AppController* c, int x, int y, int* row, int* col) {
    if (!c || !row || !col) return;

    // 获取绘图模型数据
    AppDrawModel model;
    app_controller_get_draw_model(c, &model);

    // 计算单元格宽高
    int cell_w = BOARD_WIDTH / model.cols;  // 棋盘宽度 / 列数
    int cell_h = BOARD_HEIGHT / model.rows; // 棋盘高度 / 行数

    // 计算行列号
    *col = (int)(x / cell_w);
    *row = (int)(y / cell_h);

    // 边界检查
    if (*col < 0) *col = 0;
    if (*col >= model.cols) *col = model.cols - 1;
    if (*row < 0) *row = 0;
    if (*row >= model.rows) *row = model.rows - 1;
}
void app_controll_boardToScreen_from_AppController(struct AppController* c, int row, int col, int* x, int* y) {
    if (!c || !x || !y) return;

    // 获取绘图模型数据
    AppDrawModel model;
    app_controller_get_draw_model(c, &model);

    // 计算单元格宽高
    int cell_w = BOARD_WIDTH / model.cols;  // 棋盘宽度 / 列数
    int cell_h = BOARD_HEIGHT / model.rows; // 棋盘高度 / 行数

    // 计算屏幕坐标
    *x = col * cell_w + cell_w / 2; // 单元格中心点
    *y = row * cell_h + cell_h / 2; // 单元格中心点
}
//先写固定屏幕大小的接口吧，改天研究一下支持动态棋盘大小变化的接口，估计要扩展AppDrawModel或AppController
void app_controll_ScreenToBoard(const AppDrawModel* model, int x, int y, int* row, int* col) {
    if (!model || !row || !col) return;

    // 计算单元格宽高
    int cell_w = BOARD_WIDTH / model->cols;  // 棋盘宽度 / 列数
    int cell_h = BOARD_HEIGHT / model->rows; // 棋盘高度 / 行数

    // 计算行列号
    *col = (int)(x / cell_w);
    *row = (int)(y / cell_h);

    // 边界检查
    if (*col < 0) *col = 0;
    if (*col >= model->cols) *col = model->cols - 1;
    if (*row < 0) *row = 0;
    if (*row >= model->rows) *row = model->rows - 1;
}
void app_controll_boardToScreen(const AppDrawModel* model, int row, int col, int* x, int* y) {
    if (!model || !x || !y) return;

    // 计算单元格宽高
    int cell_w = BOARD_WIDTH / model->cols;  // 棋盘宽度 / 列数
    int cell_h = BOARD_HEIGHT / model->rows; // 棋盘高度 / 行数

    // 计算屏幕坐标
    *x = col * cell_w + cell_w / 2; // 单元格中心点
    *y = row * cell_h + cell_h / 2; // 单元格中心点
}
/* --------------------------以上需要优化--------------------------------
----------------------------------------------------------------------- */