#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <platform.h>
#include "app/controller.h"
// #define BOARD_WIDTH 450
// #define BOARD_HEIGHT 510
static gboolean g_board_draw_grid = TRUE;  /* 底图有网格时改为 FALSE */
/* === 统一棋盘几何（确保缩放后棋子与棋盘对齐） === */
static double g_board_ox = 0.0, g_board_oy = 0.0; /* 棋盘左上角偏移（像素） */
static double g_board_w = 0.0, g_board_h = 0.0; /* 棋盘区域宽高（像素） */
static double g_cell = 0.0;                   /* 正方形单元格边长（像素） */

typedef struct {
    GtkWidget* window;    // 主窗口
    GtkWidget* area;      // 绘图区域（棋盘绘制载体）
    GtkWidget* header;    // 顶部按钮容器（新局/悔棋/保存/加载）
    AppController* ctrl;  // 控制器指针（关联业务层，解耦视图与模型）
    int cell_w, cell_h;   // 棋盘单元格宽高（动态计算）

    /* 贴图缓存 */
    GHashTable* pix_cache; /* key: gchar* 文件名, val: GdkPixbuf* 避免重复加载 */
} GtkChessApp;

static GtkChessApp g_app = {0};// 全局单例，管理界面状态

/* === 贴图工具 === */
static GdkPixbuf* load_piece_png(const char* name){
    if(!g_app.pix_cache){
        // 初始化缓存，指定销毁时释放key（文件名）和val（贴图）
        g_app.pix_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
    }
    // 先查缓存，命中则直接返回
    GdkPixbuf* px = (GdkPixbuf*)g_hash_table_lookup(g_app.pix_cache, name);
    if(px) return px;
    
    // 缓存未命中，拼接路径（约定：assets/gtk/pieces/<name>.png）
    gchar* path = g_strdup_printf("assets/gtk/pieces/%s.png", name);
    GError* err = NULL;
    px = gdk_pixbuf_new_from_file(path, &err); // 加载PNG图片
    if(!px){
        if(err){ pal_log_warn("png load fail %s: %s", path, err->message); g_error_free(err); }
        g_free(path);
        return NULL;
    }
    // 存入缓存，path所有权移交缓存（由缓存负责释放）
    g_hash_table_insert(g_app.pix_cache, path, px);
    return px;
}
/* === UI 贴图加载：assets/gtk/ui/<name>.png === */
static GdkPixbuf* load_ui_png(const char* name){
    if(!g_app.pix_cache){
        g_app.pix_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
    }
    // 用“路径+文件名”作为 key，避免与棋子同名冲突
    gchar* key  = g_strdup_printf("ui:%s", name);
    GdkPixbuf* px = (GdkPixbuf*)g_hash_table_lookup(g_app.pix_cache, key);
    if(px) return px;

    gchar* path = g_strdup_printf("assets/gtk/ui/%s.png", name);
    GError* err = NULL;
    px = gdk_pixbuf_new_from_file(path, &err);
    if(!px){
        if(err){ pal_log_warn("ui png load fail %s: %s", path, err->message); g_error_free(err); }
        g_free(path);
        g_free(key);
        return NULL;
    }
    g_hash_table_insert(g_app.pix_cache, key, px); // 缓存接管 key
    g_free(path);
    return px;
}
/* === 棋盘底图贴图加载：assets/gtk/board/board.png === */
static GdkPixbuf* load_board_png(void){
    if(!g_app.pix_cache){
        g_app.pix_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
    }
    const char* key = "ui:board_png";
    GdkPixbuf* px = (GdkPixbuf*)g_hash_table_lookup(g_app.pix_cache, key);
    if(px) return px;

    const char* path = "assets/gtk/board/board.png";
    GError* err = NULL;
    px = gdk_pixbuf_new_from_file(path, &err);
    if(!px){
        if(err){ pal_log_warn("board png load fail %s: %s", path, err->message); g_error_free(err); }
        return NULL;
    }
    /* key 需要复制后交给缓存托管 */
    g_hash_table_insert(g_app.pix_cache, g_strdup(key), px);
    return px;
}

/* piece code -> 文件名（不带扩展名） */
// static const char* piece_png_name(int code){
//     switch(code){
//         case R_GENERAL: return "r_general";
//         case R_ADVISOR: return "r_advisor";
//         case R_ELEPHANT:return "r_elephant";
//         case R_HORSE:   return "r_horse";
//         case R_CHARIOT: return "r_chariot";
//         case R_CANNON:  return "r_cannon";
//         case R_SOLDIER: return "r_soldier";
//         case B_GENERAL: return "b_general";
//         case B_ADVISOR: return "b_advisor";
//         case B_ELEPHANT:return "b_elephant";
//         case B_HORSE:   return "b_horse";
//         case B_CHARIOT: return "b_chariot";
//         case B_CANNON:  return "b_cannon";
//         case B_SOLDIER: return "b_soldier";
//         default: return NULL;
//     }
// }

/* 像素→格点 */

/* 像素→格点（需减去棋盘偏移） */
static void screen_to_grid(double x, double y, int* gx, int* gy){
    AppDrawModel m;
    app_controller_get_draw_model(g_app.ctrl, &m);
    // int cw = g_app.cell_w, ch = g_app.cell_h;
    // int col = (int)floor(x / cw);
    // int row = (int)floor(y / ch);
    double lx = x - g_board_ox;
    double ly = y - g_board_oy;
    int col = (int)floor(lx / g_cell);
    int row = (int)floor(ly / g_cell);
    if(col < 0) col = 0; 
    if(col >= m.cols) col = m.cols-1;
    if(row < 0) row = 0; 
    if(row >= m.rows) row = m.rows-1;
    *gx = col; *gy = row;
}
// 将chessboard[10][9]的坐标转换棋盘坐标到屏幕坐标（交叉点位置）
// chessboard棋盘坐标：(row,col) row 0~9，col 0~8
// 屏幕交叉点坐标(x,y) = (BOARD_OFFSET,BOARD_OFFSET) + (col,row) * CELL_SIZE
// static void board_to_screen(int row, int col, int* x, int* y);

// 转换屏幕坐标到chessboard[10][9]坐标（判断点击位置属于哪个交叉点）
// static int screen_to_board(int x, int y, int* row, int* col);
static void draw_palace(cairo_t* cr, int left_col, int top_row, int right_col, int bottom_row){
    double x1 = (left_col  + 0.5) * g_app.cell_w;
    double x2 = (right_col + 0.5) * g_app.cell_w;
    double y1 = (top_row   + 0.5) * g_app.cell_h;
    double y2 = (bottom_row+ 0.5) * g_app.cell_h;
    cairo_move_to(cr, x1, y1); cairo_line_to(cr, x2, y2);
    cairo_move_to(cr, x2, y1); cairo_line_to(cr, x1, y2);
    cairo_stroke(cr);
}

/* 四角 L 形选中框 */
static void draw_corner_rect(cairo_t* cr, int gx, int gy, double len_ratio, double line_w){
    double x = gx * g_app.cell_w;
    double y = gy * g_app.cell_h;
    double w = g_app.cell_w;
    double h = g_app.cell_h;
    double L = fmin(w,h) * len_ratio;
    cairo_set_line_width(cr, line_w);

    /* 上左 */
    cairo_move_to(cr, x, y+L); cairo_line_to(cr, x, y); cairo_line_to(cr, x+L, y);
    /* 上右 */
    cairo_move_to(cr, x+w-L, y); cairo_line_to(cr, x+w, y); cairo_line_to(cr, x+w, y+L);
    /* 下左 */
    cairo_move_to(cr, x, y+h-L); cairo_line_to(cr, x, y+h); cairo_line_to(cr, x+L, y+h);
    /* 下右 */
    cairo_move_to(cr, x+w-L, y+h); cairo_line_to(cr, x+w, y+h); cairo_line_to(cr, x+w, y+h-L);

    cairo_stroke(cr);
}


static gboolean on_draw(GtkWidget* w, cairo_t* cr, gpointer data){
    (void)w; (void)data;
    GtkAllocation a; 
    gtk_widget_get_allocation(w, &a); // 获取绘图区域宽高 动态调整棋盘大小

    /* 关键：统一为正方形格子 + 居中棋盘矩形 */
    g_cell     = floor(fmin(a.width / 9.0, a.height / 10.0));
    if (g_cell < 1) g_cell = 1;
    g_board_w  = g_cell * 9.0;
    g_board_h  = g_cell * 10.0;
    g_board_ox = (a.width  - g_board_w) * 0.5;
    g_board_oy = (a.height - g_board_h) * 0.5;

    /* 同步旧字段（如果别处用到了） */
    g_app.cell_w = (int)g_cell;
    g_app.cell_h = (int)g_cell;
    // g_app.cell_w = a.width / 9;  // 棋盘9列，格子宽度=区域宽度/9
    // g_app.cell_h = a.height / 10; // 棋盘10行，格子高度=区域高度/10

    AppDrawModel m; 
    app_controller_get_draw_model(g_app.ctrl, &m); // 获取棋局状态（棋子位置、选中状态等）

    /* 背景（先清屏） */
    cairo_set_source_rgb(cr, 0.98, 0.94, 0.86);
    cairo_paint(cr);

    /* 背景（优先使用自定义棋盘底图，失败回退纯色矩形） */
    {
        GdkPixbuf* board = load_board_png();
        board = NULL; ////棋盘优化还有问题，默认不打开
        if(board){
            /* 将底图缩放到棋盘矩形大小，并绘制在棋盘原点处 */
            GdkPixbuf* scaled = gdk_pixbuf_scale_simple(board, (int)g_board_w, (int)g_board_h, GDK_INTERP_BILINEAR);
            gdk_cairo_set_source_pixbuf(cr, scaled, g_board_ox, g_board_oy);
            cairo_paint(cr);
            g_object_unref(scaled);

            /* 若底图自带网格，可关闭程序网格：按需改 TRUE/FALSE */
            // g_board_draw_grid = FALSE; 
        }else{
            /* 回退：只填充棋盘区域底色 */
            cairo_set_source_rgb(cr, 0.98, 0.94, 0.86);
            cairo_rectangle(cr, g_board_ox, g_board_oy, g_board_w, g_board_h);
            cairo_fill(cr);
        }
    }
    
    /* 进入棋盘局部坐标 */
    cairo_save(cr);
    cairo_translate(cr, g_board_ox, g_board_oy);

    if (g_board_draw_grid) {
      /* 网格（中间河界断开横线） */
      cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
      cairo_set_line_width(cr, 1.2);
      // 绘制横线
      for (int r = 0; r <= 9; ++r) {
        double y = (r + 0.5) * g_app.cell_h;
        cairo_move_to(cr, g_app.cell_w * 0.5, y);            // 起点
        cairo_line_to(cr, g_board_w - g_app.cell_w * 0.5, y);// 终点（使用棋盘宽）
      }
      cairo_stroke(cr);
      // 绘制竖线
      for (int c = 0; c <= 8; ++c) {
        double x = (c + 0.5) * g_app.cell_w;

        if (c >= 1 && c <= 7) {
          // 第 1 到第 7 根竖线：在楚河汉界处断开
          // 上方部分
          cairo_move_to(cr, x, g_app.cell_h * 0.5); // 上方起点
          cairo_line_to(cr, x, g_app.cell_h * 4.5); // 上方终点（断开前）
          // 下方部分
          cairo_move_to(cr, x, g_app.cell_h * 5.5); // 下方起点（断开后）
          cairo_line_to(cr, x, g_board_h - g_app.cell_h * 0.5); // 下方终点（使用棋盘高）
        } else {
          // 第 0 根和第 8 根竖线：完整绘制
          cairo_move_to(cr, x, g_app.cell_h * 0.5);            // 起点
          cairo_line_to(cr, x, g_board_h - g_app.cell_h * 0.5); // 终点（使用棋盘高）
        }
      }
      cairo_stroke(cr);

      /* 楚河汉界（优先用 PNG，失败回退文字） */
      {
        GdkPixbuf *left_png  = load_ui_png("river_left"); // assets/gtk/ui/river_left.png
        GdkPixbuf *right_png = load_ui_png("river_right"); // assets/gtk/ui/river_right.png

        double river_center_y = (5.0) * g_app.cell_h - g_app.cell_h * 0.1; // 视觉稍微下沉一点
        double area_h = g_app.cell_h * 0.9;            // 目标高度≈1格（留边距）
        double area_w = g_app.cell_w * 3.8;            // 目标宽度≈3.8格（大致覆盖 2~4.8 列宽度）

        /* 左侧图片 —— 居中在 2.5 列附近（大致 1.5~5.5 的左半空间） */
        if (left_png) {
          int pw = gdk_pixbuf_get_width(left_png);
          int ph = gdk_pixbuf_get_height(left_png);
          double s = fmin(area_w / pw, area_h / ph);
          int dw = (int)(pw * s), dh = (int)(ph * s);

          double center_x = g_app.cell_w * 2.5;
          double x = center_x - dw / 2.0;
          double y = river_center_y - dh / 2.0;

          GdkPixbuf *scaled =
              gdk_pixbuf_scale_simple(left_png, dw, dh, GDK_INTERP_BILINEAR);
          gdk_cairo_set_source_pixbuf(cr, scaled, x, y);
          cairo_paint(cr);
          g_object_unref(scaled);
        }

        /* 右侧图片 —— 居中在 6.5 列附近（右半空间） */
        if (right_png) {
          int pw = gdk_pixbuf_get_width(right_png);
          int ph = gdk_pixbuf_get_height(right_png);
          double s = fmin(area_w / pw, area_h / ph);
          int dw = (int)(pw * s), dh = (int)(ph * s);

          double center_x = g_app.cell_w * 6.5;
          double x = center_x - dw / 2.0;
          double y = river_center_y - dh / 2.0;

          GdkPixbuf *scaled =
              gdk_pixbuf_scale_simple(right_png, dw, dh, GDK_INTERP_BILINEAR);
          gdk_cairo_set_source_pixbuf(cr, scaled, x, y);
          cairo_paint(cr);
          g_object_unref(scaled);
        }

        /* 若任一侧图片缺失，则回退到原本文字绘制 */
        if (!left_png || !right_png) {
          const char *river_left  = "楚河";
          const char *river_right = "漢界";
          cairo_select_font_face(cr, "Noto Sans CJK JP",
                                 CAIRO_FONT_SLANT_NORMAL,
                                 CAIRO_FONT_WEIGHT_BOLD);
          cairo_set_font_size(cr, g_app.cell_h * 0.35);
          cairo_set_source_rgb(cr, 0.3, 0.2, 0.1);

          cairo_text_extents_t ext;
          cairo_text_extents(cr, river_left, &ext);
          double lx = g_app.cell_w * 1.5 + ext.width / 2;
          double ly = (5) * g_app.cell_h + g_app.cell_h * 0.15;
          cairo_move_to(cr, lx, ly);
          cairo_show_text(cr, river_left);

          cairo_text_extents(cr, river_right, &ext);
          double rx = g_app.cell_w * 5.5 + ext.width / 2;
          double ry = ly;
          cairo_move_to(cr, rx, ry);
          cairo_show_text(cr, river_right);
        }
      }

      /* 九宫对角线 */
      cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
      draw_palace(cr, 3, 0, 5, 2);
      draw_palace(cr, 3, 7, 5, 9);
    }

    /* “将帅对脸”提示：整列淡红遮罩（高度按棋盘高） */
    if(m.face_to_face){
        for(int col=0; col<9; ++col){
            int generals=0;
            for(int row=0; row<10; ++row){
                int code = m.board[row][col];
                if(app_piece_is_general(code)) generals++;
            }
            if(generals==2){
                cairo_set_source_rgba(cr, 1, 0.2, 0.2, 0.18);
                cairo_rectangle(cr, col*g_app.cell_w, 0, g_app.cell_w, g_board_h);
                cairo_fill(cr);
                break;
            }
        }
    }

    /* “被将”提示：给对应的将/帥画红色角标框 */
    if(m.in_check_red || m.in_check_black){
        for(int r=0;r<10;r++){
            for(int c=0;c<9;c++){
                int code = m.board[r][c];
                if(!app_piece_is_general(code)) continue;

                AppPieceColor col = app_piece_color(code);
                if( (m.in_check_red   && col == APP_COLOR_RED) ||
                    (m.in_check_black && col == APP_COLOR_BLACK) ){
                    cairo_set_source_rgb(cr, 0.85, 0.1, 0.1);
                    draw_corner_rect(cr, c, r, 0.22, 2.2);
                }
            }
        }
    }


    /* 选中角标框 */
    if(m.selected_x>=0){
        cairo_set_source_rgb(cr, 1, 0.7, 0.15);
        draw_corner_rect(cr, m.selected_x, m.selected_y, 0.25, 1.8);
    }

    /* 棋子（优先 PNG，不存在则退回文字） */
    for(int r=0;r<m.rows;r++){
        for(int c=0;c<m.cols;c++){
            int code = m.board[r][c];
            if(code == 0) continue;

            const char* png = app_piece_png_id(code); 
            GdkPixbuf* px = png ? load_piece_png(png) : NULL;
            double cx = c * g_app.cell_w;
            double cy = r * g_app.cell_h;
            if(px){
                /* 等比缩放到 cell 内（留点边距） */
                int pw = gdk_pixbuf_get_width(px);
                int ph = gdk_pixbuf_get_height(px);
                double scale = fmin((g_app.cell_w*0.9)/pw, (g_app.cell_h*0.9)/ph);
                int dw = (int)(pw * scale);
                int dh = (int)(ph * scale);
                GdkPixbuf* scaled = gdk_pixbuf_scale_simple(px, dw, dh, GDK_INTERP_BILINEAR);
                gdk_cairo_set_source_pixbuf(cr, scaled,
                    cx + (g_app.cell_w-dw)/2.0,
                    cy + (g_app.cell_h-dh)/2.0);
                cairo_paint(cr);
                g_object_unref(scaled);
            }else{
                /* 退回文字渲染 */
                const char* label = app_piece_label(code);
                AppPieceColor col = app_piece_color(code);
                cairo_select_font_face(cr, "Noto Sans CJK JP", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
                cairo_set_font_size(cr, g_app.cell_h * 0.55);
                if(col == APP_COLOR_RED) cairo_set_source_rgb(cr, 0.8, 0.1, 0.1);
                else                     cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
                cairo_move_to(cr, cx + g_app.cell_w*0.35, cy + g_app.cell_h*0.7);
                cairo_show_text(cr, label);
            }
        }
    }

    /* 可走提示点 */
    cairo_set_source_rgba(cr, 0.1, 0.6, 0.2, 0.85);
    for(int i=0;i<m.legal_count;i++){
        int tx = m.legal[i].tx;
        int ty = m.legal[i].ty;
        double cx = tx * g_app.cell_w + g_app.cell_w*0.5;
        double cy = ty * g_app.cell_h + g_app.cell_h*0.5;
        double r  = g_app.cell_w * 0.12;
        cairo_arc(cr, cx, cy, r, 0, 2*M_PI);
        cairo_fill(cr);
    }

    cairo_restore(cr);
    return TRUE;
}

static gboolean on_button_press(GtkWidget* w, GdkEventButton* e, gpointer ud){
    (void)w; (void)ud;
    if(e->type == GDK_BUTTON_PRESS && e->button == 1){
        int gx, gy; 
        #if 1
        screen_to_grid(e->x, e->y, &gx, &gy);
        #elif

        // 获取绘图模型数据
        AppDrawModel model;
        app_controller_get_draw_model(g_app.ctrl, &model);
        // 调用优化后的坐标转换函数
        app_controll_ScreenToBoard(&model, (int)e->x, (int)e->y, &gy, &gx);
        #endif
        // 处理点击事件
        app_controller_click_cell(g_app.ctrl, gx, gy);
        gtk_widget_queue_draw(g_app.area);
    }
    return TRUE;
}

/* 顶部按钮 */
static void on_new_clicked(GtkButton* b, gpointer ud){
    (void)b; (void)ud;
    app_controller_new_game(g_app.ctrl);
    gtk_widget_queue_draw(g_app.area);
}
static void on_undo_clicked(GtkButton* b, gpointer ud){
    (void)b; (void)ud;
    if(app_controller_undo(g_app.ctrl)==0){
        gtk_widget_queue_draw(g_app.area);
    }else{
        GtkWidget* dialog = gtk_message_dialog_new(
            GTK_WINDOW(g_app.window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "无法悔棋：没有可撤销的记录或历史数据异常。"
        );
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}
static void on_save_clicked(GtkButton* b, gpointer ud){
    (void)b; (void)ud;
    GtkWidget* dlg = gtk_file_chooser_dialog_new("保存棋局",
        GTK_WINDOW(g_app.window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_取消", GTK_RESPONSE_CANCEL,
        "_保存", GTK_RESPONSE_ACCEPT, NULL);
    if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT){
        char* fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        if(app_controller_save(g_app.ctrl, fn)==0){
            pal_log_info("Saved: %s", fn);
        }else{
            pal_log_error("Save failed: %s", fn);
        }
        g_free(fn);
    }
    gtk_widget_destroy(dlg);
}
static void on_load_clicked(GtkButton* b, gpointer ud){
    (void)b; (void)ud;
    GtkWidget* dlg = gtk_file_chooser_dialog_new("加载棋局",
        GTK_WINDOW(g_app.window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_取消", GTK_RESPONSE_CANCEL,
        "_打开", GTK_RESPONSE_ACCEPT, NULL);
    if(gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT){
        char* fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        if(app_controller_load(g_app.ctrl, fn)==0){
            pal_log_info("Loaded: %s", fn);
            gtk_widget_queue_draw(g_app.area);
        }else{
            pal_log_error("Load failed: %s", fn);
        }
        g_free(fn);
    }
    gtk_widget_destroy(dlg);
}

int gtk_chess_main(int argc, char** argv){
    gtk_init(&argc, &argv);

    g_app.ctrl = app_controller_create(); //创建一个全局控制器实例

    // 创建一个顶层窗口
    g_app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(g_app.window), "Chinese Chess");
    gtk_window_set_default_size(GTK_WINDOW(g_app.window), BOARD_WIDTH, BOARD_HEIGHT);
    // 创建功能按钮
    GtkWidget* box_vert = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);// 垂直布局容器
    g_app.header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);// 水平布局容器
    GtkWidget* btn_new  = gtk_button_new_with_label("新局");
    GtkWidget* btn_undo = gtk_button_new_with_label("悔棋");
    GtkWidget* btn_save = gtk_button_new_with_label("保存");
    GtkWidget* btn_load = gtk_button_new_with_label("加载");
    g_signal_connect(btn_new,  "clicked", G_CALLBACK(on_new_clicked),  NULL);
    g_signal_connect(btn_undo, "clicked", G_CALLBACK(on_undo_clicked), NULL);
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), NULL);
    g_signal_connect(btn_load, "clicked", G_CALLBACK(on_load_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(g_app.header), btn_new,  FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(g_app.header), btn_undo, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(g_app.header), btn_save, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(g_app.header), btn_load, FALSE, FALSE, 4);
    // 创建绘图区域
    g_app.area = gtk_drawing_area_new();//创建一个绘图区域 g_app.area，用于绘制棋盘和棋子
    gtk_widget_add_events(g_app.area, GDK_BUTTON_PRESS_MASK);//添加鼠标点击事件监听器
    g_signal_connect(g_app.area, "draw", G_CALLBACK(on_draw), NULL);//绘图事件处理函数 on_draw，用于绘制棋盘和棋子
    g_signal_connect(g_app.area, "button-press-event", G_CALLBACK(on_button_press), NULL);//鼠标点击事件处理函数 on_button_press，用于响应用户的点击操作

    g_signal_connect(g_app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);//绑定标准退出函数

    gtk_box_pack_start(GTK_BOX(box_vert), g_app.header, FALSE, FALSE, 4);
    gtk_box_pack_end  (GTK_BOX(box_vert), g_app.area,   TRUE,  TRUE,  0);
    gtk_container_add(GTK_CONTAINER(g_app.window), box_vert);

    // 显示所有控件并启动GTK主循环
    gtk_widget_show_all(g_app.window);
    gtk_main();

    /* 释放贴图缓存 */
    if(g_app.pix_cache){ g_hash_table_destroy(g_app.pix_cache); g_app.pix_cache=NULL; }

    app_controller_destroy(g_app.ctrl);
    return 0;
}


