#include <stdio.h>
#include <string.h>
#include <chess.h>
#include <stdlib.h>
/* Stub file for future move history / undo / serialization splitting.
   For now, we intentionally keep logic in chess.c; this file ensures
   the project structure can grow without breaking build. */
void chess_stub_move_module_linker_anchor(void) {}
struct Move;
struct MoveList
{
   struct Move* moves;
   int count;  //有效走法数量
   int capacity;
};

struct MoveList* MoveList_create(int capacity){
   struct MoveList* list = calloc(1, sizeof(struct MoveList));
   if(list == NULL) return NULL;

   list->capacity = capacity;
   list->count = 0;
   list->moves = calloc(1,sizeof(struct Move)*list->capacity);
   return list;
}
static void MoveList_push(struct MoveList* list, struct Move move){
   if(list == NULL) return;

   if (list->count >= list->capacity){
      list->capacity = list->capacity * 2;
      list->moves = realloc(list->moves, sizeof(struct Move)*list->capacity);
   }
   list->moves[list->count++] = move;// 存入新走法
}
void MoveList_destroy(struct MoveList* list) {
    if (list == NULL) return;
    free(list->moves);  // 先释放内部数组
    free(list);        // 再释放列表本身
}

struct MoveList* Chessboard_getPossibleMoves(struct Chessboard* cb, struct Position from, enum Player player) {
    if (cb == NULL) return NULL;

    if (!isValidPosition(from)) return MoveList_create(0); 

    // 获取起始位置的棋子
    enum PieceType movingPiece = Chessboard_getPiece(cb, from);
    // 检查是否有棋子，且属于当前玩家
    if (movingPiece == EMPTY) return MoveList_create(0);
    if ((player == RED && !isRed(movingPiece)) || (player == BLACK && !isBlack(movingPiece))) return MoveList_create(0);

    // 创建走法列表
    struct MoveList* possibleMoves = MoveList_create(5);
    if (possibleMoves == NULL) return NULL;

    // 遍历棋盘所有可能的目标位置（10行9列）
    for (int to_row = 0; to_row < 10; to_row++) {
        for (int to_col = 0; to_col < 9; to_col++) {
            // 跳过起始位置本身（无意义的移动）
            if (to_row == from.row && to_col == from.col) {
                continue;
            }

            // 构造走法结构体
            struct Move possibleMove = {
                .from = from,
                .to = {to_row, to_col},
                .captured = Chessboard_getPiece(cb, (struct Position){to_row, to_col}),  // 记录被吃的棋子
                .steps = -1,  // 暂未使用，设为-1
                .is_checked = false  // 初始设为未将军
            };

            // 检查走法是否有效
            if (Chessboard_isValidMoveQuiet(cb, possibleMove, player)) {
                MoveList_push(possibleMoves, possibleMove);  // 有效则加入列表
            }
        }
    }

    return possibleMoves;
}