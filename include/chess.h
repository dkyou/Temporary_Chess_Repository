#ifndef CHESS_H
#define CHESS_H

#include <stdbool.h>

// 棋子类型枚举
enum PieceType {
    EMPTY,
    R_GENERAL,   // 红帅
    R_ADVISOR,   // 红士
    R_ELEPHANT,  // 红相
    R_HORSE,     // 红马
    R_CHARIOT,   // 红车
    R_CANNON,    // 红炮
    R_SOLDIER,   // 红兵
    B_GENERAL,   // 黑将
    B_ADVISOR,   // 黑士
    B_ELEPHANT,  // 黑象
    B_HORSE,     // 黑马
    B_CHARIOT,   // 黑车
    B_CANNON,    // 黑炮
    B_SOLDIER    // 黑卒
};

// 玩家颜色
enum Player {
    RED,
    BLACK
};

// 位置结构体
struct Position {
    int row;
    int col;
};

// 走法结构体
struct Move {
    struct Position from;
    struct Position to;
    int steps;//记录双方走棋的步数和，暂未使用
    enum PieceType captured;  // 被吃掉的棋子
    bool is_checked;
};

// 走法历史，用于悔棋
struct MoveHistory {
    struct Move* moves;//记录上一次走棋
    int count;      //悔棋次数
    int capacity;   //剩余悔棋次数
};

struct Chessboard {
    enum PieceType board[10][9];  // 10行9列棋盘
};

// 游戏结构体---核心
struct Game {
    struct Chessboard* chessboard;
    struct MoveHistory history;
    enum Player currentPlayer;
    bool gameOver;
    //GUI状态相关---GTK使用
    struct Position selectedPos;//选中棋子的位置
    bool isPieceSelected;       //是否有棋子被选中
};

/* 拓展结构体 */
// 选中棋子的信息结构体（包含合法性和核心数据）--目前用于构造getSelectedPieceInfo 但暂未实际使用
// 减少重复编写 “坐标校验 + 棋子类型读取 + 归属判断” 的代码
struct SelectedPieceInfo {
    bool isValid;          // 选中是否有效（满足坐标+有棋子+己方）
    enum PieceType type;   // 棋子类型（如R_CHARIOT、B_HORSE，无效时为EMPTY）
    struct Position pos;   // 选中棋子的坐标（无效时为{-1,-1}）
    enum Player owner;     // 棋子所属玩家（无效时为RED/BLACK均可，需结合isValid判断）
};

// 核心逻辑函数声明
void Chessboard_initialize(struct Chessboard* cb);
void Game_initialize(struct Game* game);
bool Game_saveGame(struct Game* game, const char* filename);
bool Game_loadGame(struct Game* game, const char* filename);

bool Chessboard_isValidMove(struct Chessboard* cb, struct Move move, enum Player player);
bool Chessboard_isValidMoveQuiet(struct Chessboard* cb, struct Move move, enum Player player);
bool Chessboard_isInCheck(struct Chessboard* cb, enum Player player);
bool Chessboard_isCheckmated(struct Chessboard* cb, enum Player player);//not use
bool doesMoveResolveCheck(struct Chessboard* cb, struct Move move, enum Player player);
bool willMoveCauseCheck(struct Chessboard* cb, struct Move move, enum Player player);


bool Chessboard_makeMove(struct Chessboard* cb, struct Move move, struct MoveHistory* history);
void Chessboard_undoMove(struct Chessboard* cb, struct MoveHistory* history);
bool Chessboard_undoMove_enhance(struct Chessboard* cb, struct MoveHistory* history);
enum PieceType Chessboard_getPiece(struct Chessboard* cb, struct Position pos);
enum PieceType Chessboard_getPieceAt(struct Chessboard* cb, int row, int col);
const char* Chessboard_getPieceText(enum PieceType piece);
void Chessboard_setPiece(struct Chessboard* cb, struct Position pos, enum PieceType piece);
void Chessboard_showPossibleMoves(struct Chessboard* cb, struct Position pos, enum Player player);

// 辅助函数 定义1：is 0:not is
bool isRed(enum PieceType piece);
bool isBlack(enum PieceType piece);
bool isSameColor(enum PieceType a, enum PieceType b);
bool isInPalace(struct Position pos, enum Player player);
bool isOwnPiece(enum Player player, enum PieceType piece);
bool isGeneral(enum PieceType piece);
bool isStraightPathClear(struct Chessboard* cb, struct Position from, struct Position to);
inline bool isValidPosition(struct Position pos){
    return pos.row >= 0 && pos.row < 10 && pos.col >= 0 && pos.col < 9;
}
bool hasAnyValidMoves(struct Chessboard* cb, enum Player player);
void Game_switchPlayer(struct Game* game);
struct Position findGeneral(struct Chessboard* cb, enum Player player);
const char* Chessboard_PieceToText(enum PieceType piece);
char Chessboard_pieceToChar(enum PieceType piece);
struct SelectedPieceInfo getSelectedPieceInfo(struct Chessboard* cb, struct Position selectedPos, enum Player currentPlayer);
// 新增接口（供LVGL调用）

enum Player Game_getCurrentPlayer(struct Game* game);
bool Game_isOver(struct Game* game);
void Game_undo(struct Game* game);  // 悔棋接口
/*
*********************************用于支持终端运行*****************************************
*/
void GameRun_InTerminal(struct Game* game); 
void ChessboardDisplay_InTerminal(struct Chessboard* cb);
void GameRun_Terminal_main(void);
#endif
/*
*********************************用于支持终端运行*****************************************
*/

/*说明：棋盘结构体 棋子位于棋盘的交叉处*/
/*棋盘原始数据结构如下：
  0 1 2 3 4 5 6 7 8
 +-+-+-+-+-+-+-+-+-+
0|c|h|e|a|k|a|e|h|c|
 +-+-+-+-+-+-+-+-+-+
1| | | | | | | | | |
 +-+-+-+-+-+-+-+-+-+
2| |n| | | | | |n| |
 +-+-+-+-+-+-+-+-+-+
3|s| |s| |s| |s| |s|
 +-+-+-+-+-+-+-+-+-+
4| | | | | | | | | |
 +-+-+-+-+-+-+-+-+-+
5| | | | | | | | | |
 +-+-+-+-+-+-+-+-+-+
6|S| |S| |S| |S| |S|
 +-+-+-+-+-+-+-+-+-+
7| |N| | | | | |N| |
 +-+-+-+-+-+-+-+-+-+
8| | | | | | | | | |
 +-+-+-+-+-+-+-+-+-+
9|C|H|E|A|K|A|E|H|C|
 +-+-+-+-+-+-+-+-+-+
*/
/*
gtk 坐标系为：左上角为原点，X 轴向右，Y 轴向下
(0,0)
------------------------->x
|
|
|
|
|
|
|
|
v y 
*/