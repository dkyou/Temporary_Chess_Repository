#include <stdbool.h>
#ifdef __linux__
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "chess.h"
/*
***************实现象棋的基本逻辑***************
*/
// 棋盘初始化 棋子位置对应交叉点坐标
void Chessboard_initialize(struct Chessboard* cb) {
    // 初始化棋盘为空
    memset(cb->board, EMPTY, sizeof(cb->board));

    // 放置红方棋子
    cb->board[9][0] = R_CHARIOT;
    cb->board[9][1] = R_HORSE;
    cb->board[9][2] = R_ELEPHANT;
    cb->board[9][3] = R_ADVISOR;
    cb->board[9][4] = R_GENERAL;
    cb->board[9][5] = R_ADVISOR;
    cb->board[9][6] = R_ELEPHANT;
    cb->board[9][7] = R_HORSE;
    cb->board[9][8] = R_CHARIOT;
    cb->board[7][1] = R_CANNON;
    cb->board[7][7] = R_CANNON;
    cb->board[6][0] = R_SOLDIER;
    cb->board[6][2] = R_SOLDIER;
    cb->board[6][4] = R_SOLDIER;
    cb->board[6][6] = R_SOLDIER;
    cb->board[6][8] = R_SOLDIER;
    
    // 放置黑方棋子
    cb->board[0][0] = B_CHARIOT;
    cb->board[0][1] = B_HORSE;
    cb->board[0][2] = B_ELEPHANT;
    cb->board[0][3] = B_ADVISOR;
    cb->board[0][4] = B_GENERAL;
    cb->board[0][5] = B_ADVISOR;
    cb->board[0][6] = B_ELEPHANT;
    cb->board[0][7] = B_HORSE;
    cb->board[0][8] = B_CHARIOT;
    cb->board[2][1] = B_CANNON;
    cb->board[2][7] = B_CANNON;
    cb->board[3][0] = B_SOLDIER;
    cb->board[3][2] = B_SOLDIER;
    cb->board[3][4] = B_SOLDIER;
    cb->board[3][6] = B_SOLDIER;
    cb->board[3][8] = B_SOLDIER;
}

enum PieceType Chessboard_getPiece(struct Chessboard* cb, struct Position pos) {
    if (!isValidPosition(pos)) {
        return EMPTY;
    }
    return cb->board[pos.row][pos.col];
}

void Chessboard_setPiece(struct Chessboard* cb, struct Position pos, enum PieceType piece) {
    if (isValidPosition(pos)) {
        cb->board[pos.row][pos.col] = piece;
    }
}

// 获取指定行/列的棋子类型
enum PieceType Chessboard_getPieceAt(struct Chessboard* cb, int row, int col) {
    return Chessboard_getPiece(cb, (struct Position){row, col});
}
// 获取棋子的文本表示（如"帅"、"将"、"车"等）
const char* Chessboard_PieceToText(enum PieceType piece) {
    switch (piece) {
        case R_GENERAL: return "帅";
        case R_ADVISOR: return "仕";
        case R_ELEPHANT: return "相";
        case R_HORSE: return "马";
        case R_CHARIOT: return "车";
        case R_CANNON: return "炮";
        case R_SOLDIER: return "兵";
        case B_GENERAL: return "将";
        case B_ADVISOR: return "士";
        case B_ELEPHANT: return "象";
        case B_HORSE: return "马";
        case B_CHARIOT: return "车";
        case B_CANNON: return "炮";
        case B_SOLDIER: return "卒";
        default: 
            printf("%s %d Invalid piece type: %d\n", __func__,__LINE__,piece);
            return "";
    }
}
// 获取棋子的字符串表示（如"K"、"k"、"H"等）
char Chessboard_pieceToChar(enum PieceType piece) {
    switch (piece) {
        case EMPTY:       return ' ';
        case R_GENERAL:   return 'K';
        case R_ADVISOR:   return 'A';
        case R_ELEPHANT:  return 'E';
        case R_HORSE:     return 'H';
        case R_CHARIOT:   return 'C';
        case R_CANNON:    return 'N';
        case R_SOLDIER:   return 'S';
        case B_GENERAL:   return 'k';
        case B_ADVISOR:   return 'a';
        case B_ELEPHANT:  return 'e';
        case B_HORSE:     return 'h';
        case B_CHARIOT:   return 'c';
        case B_CANNON:    return 'n';
        case B_SOLDIER:   return 's';
        default:
            printf("%s %d Invalid piece type: %d\n", __func__,__LINE__,piece);
            return '?';
    }
}
//走棋核心逻辑
bool isBasicMoveValid(struct Chessboard* cb, struct Move move, enum Player player, enum PieceType movingPiece) {
    // 推荐顺序：
    // 1. 边界检查
    // 2. 是否为空移动？ → if (move.from.row == move.to.row && move.from.col == move.to.col) return false;
    // 3. 目标位置是否己方棋子
    // 4. movingPiece 是否合法？ → if (!isValidPiece(movingPiece)) return false;
    // 检查起始位置和目标位置是否在棋盘范围内
    if (move.from.row < 0 || move.from.row >= 10 || move.from.col < 0 || move.from.col >= 9 ||
        move.to.row < 0 || move.to.row >= 10 || move.to.col < 0 || move.to.col >= 9) {
        return false;
    }
    // 检查是否不移动
    if (move.from.row == move.to.row && move.from.col == move.to.col) {
        return false;
    }
    // 检查目标位置是否有己方棋子
    enum PieceType targetPiece = cb->board[move.to.row][move.to.col];
    if (isSameColor(movingPiece, targetPiece)) return false;
    
    int rowDiff = abs(move.to.row - move.from.row);
    int colDiff = abs(move.to.col - move.from.col);
    
    switch (movingPiece) {
        case R_GENERAL:
        case B_GENERAL:
            //检查目标位置是否在当前玩家的九宫格内,如果不在，并且是不是飞将，则返回false
            if (!isInPalace(move.to, player) && !(rowDiff == 0 && colDiff > 1)) return false;
            if ((rowDiff == 1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1)) return true;
            if (rowDiff == 0 && colDiff > 1) {
                //飞将
                return isStraightPathClear(cb, move.from, move.to) && isGeneral(targetPiece);
            }
            return false;
            
        case R_ADVISOR:
        case B_ADVISOR:
            return isInPalace(move.to, player) && rowDiff == 1 && colDiff == 1;
            
        case R_ELEPHANT:
        case B_ELEPHANT:
            if (rowDiff != 2 || colDiff != 2) return false;
            if ((movingPiece == R_ELEPHANT && move.to.row < 5) || 
                (movingPiece == B_ELEPHANT && move.to.row > 4)) return false;
            struct Position center = {
                move.from.row + (move.to.row - move.from.row)/2,
                move.from.col + (move.to.col - move.from.col)/2
            };
            return cb->board[center.row][center.col] == EMPTY;
            
        case R_HORSE:
        case B_HORSE:
            if (!((rowDiff == 1 && colDiff == 2) || (rowDiff == 2 && colDiff == 1))) return false;
            struct Position leg;
            if (rowDiff == 2) {
                leg.row = move.from.row + (move.to.row > move.from.row ? 1 : -1);
                leg.col = move.from.col;
            } else {
                leg.row = move.from.row;
                leg.col = move.from.col + (move.to.col > move.from.col ? 1 : -1);
            }
            return cb->board[leg.row][leg.col] == EMPTY;
            
        case R_CHARIOT:
        case B_CHARIOT:
            return (rowDiff == 0 || colDiff == 0) && isStraightPathClear(cb, move.from, move.to);
            
        case R_CANNON:
        case B_CANNON:
            if (rowDiff != 0 && colDiff != 0) return false;
            if (targetPiece == EMPTY) return isStraightPathClear(cb, move.from, move.to);
            int obstacles = 0;
            int stepRow = (move.to.row > move.from.row) ? 1 : (move.to.row < move.from.row) ? -1 : 0;
            int stepCol = (move.to.col > move.from.col) ? 1 : (move.to.col < move.from.col) ? -1 : 0;
            struct Position pos = move.from;
            while (1) {
                pos.row += stepRow;
                pos.col += stepCol;
                if (pos.row == move.to.row && pos.col == move.to.col) break;
                if (cb->board[pos.row][pos.col] != EMPTY) obstacles++;
            }
            return obstacles == 1;
            
        case R_SOLDIER:
            if (move.to.row > move.from.row) return false;
            if (rowDiff + colDiff != 1) return false;
            return !(move.from.row > 4 && colDiff != 0);
            
        case B_SOLDIER:
            if (move.to.row < move.from.row) return false;
            if (rowDiff + colDiff != 1) return false;
            return !(move.from.row < 5 && colDiff != 0);
            
        default:
            return false;
    }
}

/* 优化思路：
*  根据将的位置，反向生成所有可能吃掉它的攻击方式
// bool canOpponentCaptureGeneral(struct Chessboard* cb, enum Player currentPlayer) {
//     enum Player opponent = (currentPlayer == RED) ? BLACK : RED;
//     struct Position kingPos = findGeneral(cb, currentPlayer);
//     if (kingPos.row == -1) return false;

//     // 分别检查每种可能吃将的棋子类型能否攻击该位置
//     return 
//         canRookAttackSquare(cb, kingPos, opponent) ||
//         canHorseAttackSquare(cb, kingPos, opponent) ||
//         canCannonAttackSquare(cb, kingPos, opponent) ||
//         canPawnAttackSquare(cb, kingPos, opponent) ||
//         canMinisterAttackSquare(cb, kingPos, opponent) ||  // 象
//         canAdvisorAttackSquare(cb, kingPos, opponent) ||  // 士
//         canKingAttackSquare(cb, kingPos, opponent);       // 帅照面
// }
**/
//检测对手所有的棋子是否能够吃掉当前玩家(currentPlayer)的将/帅
bool canOpponentCaptureGeneral(struct Chessboard* cb, enum Player currentPlayer) {
    enum Player opponent = (currentPlayer == RED) ? BLACK : RED;
    struct Position generalPos = findGeneral(cb, currentPlayer);
    if (generalPos.row == -1) return false;
    
    for (int r = 0; r < 10; r++) {
        for (int c = 0; c < 9; c++) {
            struct Position from = {r, c};
            enum PieceType piece = cb->board[r][c];

            if ((opponent == RED && isBlack(piece)) || (opponent == BLACK && isRed(piece))) continue;
            // if ((opponent == RED && !isRed(piece)) || (opponent == BLACK && !isBlack(piece))) continue;
            // struct Move captureMove = {from, generalPos, EMPTY};
            struct Move captureMove = {0};//消除警告
            captureMove.from = from;
            captureMove.to = generalPos;
            captureMove.captured = EMPTY;
            if (isBasicMoveValid(cb, captureMove, opponent, piece)) return true;
        }
    }
    return false;
}
bool Chessboard_isInCheck(struct Chessboard* cb, enum Player player) {
    return canOpponentCaptureGeneral(cb, player);
}
// 走法有效性检查, 只是检查走法是否合法，而不会真正移动棋子
bool Chessboard_isValidMove(struct Chessboard* cb, struct Move move, enum Player player) {
    enum PieceType movingPiece = Chessboard_getPiece(cb,move.from);
    //检查移动棋子的前置条件：1.移动的棋子必须存在 2.移动的棋子必须属于当前玩家
    if(movingPiece == EMPTY){
        printf("Invalid move: no piece at source position\n");
        return false;
    }
    if((player == RED && !isRed(movingPiece)) || (player == BLACK && !isBlack(movingPiece))){
        printf("Invalid move: wrong player\n");
        return false;
    }

    //移动棋子前检查是否正在被将军，如果正在被将军必须解将，否则返回0
    bool isInCheck = Chessboard_isInCheck(cb, player);
    if(!isBasicMoveValid(cb, move, player,movingPiece)){
        printf("Invalid move: not a basic move\n");
        return false;
    } 
    //1.正在被将军，判断移动能够解将则返回1，不能解将返回0
    //2.没被将军，判断移动导致自己被将军返回0，不会导致自己被将军返回1
    if(isInCheck) return doesMoveResolveCheck(cb, move, player);
    else return !willMoveCauseCheck(cb, move, player);

}
// 静默版本，暂时先增加一个静默版本解决将军时的大量Invalid move: not a basic move输出
bool Chessboard_isValidMoveQuiet(struct Chessboard* cb, struct Move move, enum Player player) {
    enum PieceType movingPiece = Chessboard_getPiece(cb,move.from);
    //检查移动棋子的前置条件：1.移动的棋子必须存在 2.移动的棋子必须属于当前玩家
    if(movingPiece == EMPTY) return false;

    // from/to位置是否在棋盘范围内（避免越界访问）
    if (!isValidPosition(move.from) || !isValidPosition(move.to)) return false;

    // 移动棋子是否属于当前玩家
    if(!isOwnPiece(player,movingPiece)) return false;

    //移动棋子前检查是否正在被将军，如果正在被将军必须解将，否则返回0
    bool isInCheck = Chessboard_isInCheck(cb, player);
    if(!isBasicMoveValid(cb, move, player,movingPiece)) return false;

    //1.正在被将军，判断移动能够解将则返回1，不能解将返回0
    //2.没被将军，判断移动导致自己被将军返回0，不会导致自己被将军返回1
    if(isInCheck) return doesMoveResolveCheck(cb, move, player);
    else return !willMoveCauseCheck(cb, move, player);

}
//移动棋子
bool Chessboard_makeMove(struct Chessboard* cb, struct Move move, struct MoveHistory* history) {
    enum PieceType movingPiece = Chessboard_getPiece(cb, move.from);
    enum Player player = isRed(movingPiece) ? RED : BLACK;
    
    if (!Chessboard_isValidMove(cb, move, player)) {
        printf("%sInvalid move: (%d,%d) -> (%d,%d)\n", \
        player == RED ? "Red" : "Black",move.from.row,move.from.col, move.to.row,move.to.col);
        return false;
    }
    move.captured = Chessboard_getPiece(cb, move.to);
    Chessboard_setPiece(cb, move.to, movingPiece);
    Chessboard_setPiece(cb, move.from, EMPTY);
    // printf("记录走棋：%d步\n", history->count);

    if (history->count >= history->capacity) {
        printf("扩容：%d -> %d\n", history->capacity, history->capacity + 10);
        history->capacity += 10;
        history->moves = realloc(history->moves, history->capacity * sizeof(struct Move));
        if (history->moves == NULL){
            fprintf(stderr, "内存分配失败：无法记录走棋\n");
            return false;
        }
    }
    history->moves[history->count++] = move;//记录当前步
    printf("走棋记录：第%d步(从(%d,%d)到(%d,%d)\n", 
       history->count, 
       move.from.row, move.from.col,
       move.to.row, move.to.col);
    return true;
}

void Chessboard_undoMove(struct Chessboard* cb, struct MoveHistory* history) {
    if (history->count <= 0){
        printf("没有可撤销的步数\n");
        return;
    }
    struct Move lastMove = history->moves[--history->count];
    enum PieceType movedPiece = Chessboard_getPiece(cb, lastMove.to);
    Chessboard_setPiece(cb, lastMove.from, movedPiece);
    Chessboard_setPiece(cb, lastMove.to, lastMove.captured);
}


// 安全版悔棋函数，增加了一些条件判断，
// 并且增加了一些悔棋后的处理，可以使得调用者完全恢复到上一步而不需要做额外的处理
bool Chessboard_undoMove_enhance(struct Chessboard* cb, struct MoveHistory* history) {
    // 条件检查
    if (cb == NULL || history == NULL || history->moves == NULL || history->count <= 0) {
        printf("%s %d 无法撤销走棋\n",__func__,__LINE__);
        return false;
    }

    int lastIndex = history->count - 1;
    struct Move lastMove = history->moves[lastIndex];

    // 检查棋盘坐标有效性（防止越界访问棋盘数组）
    if (lastMove.from.row < 0 || lastMove.from.row >= 10 ||
        lastMove.from.col < 0 || lastMove.from.col >= 9 ||
        lastMove.to.row < 0 || lastMove.to.row >= 10 ||
        lastMove.to.col < 0 || lastMove.to.col >= 9) {
        fprintf(stderr, "错误：无效的棋盘坐标，无法撤销\n");
        history->count--;  // 跳过错误记录
        return false;
    }

    // 恢复棋子 
    // 其实可以将Chessboard_undoMove嵌入其中，后期统一风格优化代码再考虑吧
    enum PieceType movedPiece = Chessboard_getPiece(cb, lastMove.to);
    Chessboard_setPiece(cb, lastMove.from, movedPiece);
    Chessboard_setPiece(cb, lastMove.to, lastMove.captured);

    // 减少计数
    history->count--;

    //切换玩家 需要调用者处理

    return true;
}
    

bool Chessboard_isCheckmated(struct Chessboard* cb, enum Player player) {
    enum Player opponent = (player == RED) ? BLACK : RED;
    struct Position generalPos = {-1, -1};
    enum PieceType target = (player == RED) ? R_GENERAL : B_GENERAL;
    
    for (int r = 0; r < 10; r++) {
        for (int c = 0; c < 9; c++) {
            if (cb->board[r][c] == target) {
                generalPos.row = r;
                generalPos.col = c;
                break;
            }
        }
        if (generalPos.row != -1) break;
    }
    
    for (int r = 0; r < 10; r++) {
        for (int c = 0; c < 9; c++) {
            struct Position from = {r, c};
            enum PieceType piece = Chessboard_getPiece(cb, from);
            if ((opponent == RED && isRed(piece)) || (opponent == BLACK && isBlack(piece))) {
                // struct Move move = {from, generalPos, EMPTY};
                struct Move move = {0};//消除警告
                move.from = from;
                move.to = generalPos;
                move.captured = EMPTY;
                if (Chessboard_isValidMove(cb, move, opponent)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

// 游戏初始化
void Game_initialize(struct Game* game) {
    game->chessboard = malloc(sizeof(struct Chessboard));
    Chessboard_initialize(game->chessboard);
    
    // 初始化历史记录（关键：确保moves不为NULL）
    game->history.count = 0;
    game->history.capacity = 20;  // 初始容量设大一些，避免频繁扩容
    game->history.moves = malloc(game->history.capacity * sizeof(struct Move));
    if (game->history.moves == NULL) {
        fprintf(stderr, "内存分配失败：无法初始化历史记录\n");
        exit(1);  // 分配失败时退出，避免后续崩溃
    }
    
    game->currentPlayer = RED;
    game->gameOver = false;
    game->isPieceSelected = false;
    game->selectedPos.row = -1;
    game->selectedPos.col = -1;
}

void Game_switchPlayer(struct Game* game) {
    game->currentPlayer = (game->currentPlayer == RED) ? BLACK : RED;
}

// 获取当前玩家
enum Player Game_getCurrentPlayer(struct Game* game) {
    return game->currentPlayer;
}

// 判断游戏是否结束
bool Game_isOver(struct Game* game) {
    return game->gameOver;
}

// 悔棋功能
void Game_undo(struct Game* game) {
    if (game->history.count >= 2) {
        Chessboard_undoMove(game->chessboard, &game->history);
        Chessboard_undoMove(game->chessboard, &game->history);
    }
}

bool Game_saveGame(struct Game* game, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) return false;
    
    for (int i = 0; i < 10; i++) {
        fwrite(game->chessboard->board[i], sizeof(enum PieceType), 9, file);
    }
    fwrite(&game->currentPlayer, sizeof(enum Player), 1, file);
    fwrite(&game->history.count, sizeof(int), 1, file);
    fwrite(game->history.moves, sizeof(struct Move), game->history.count, file);
    
    fclose(file);
    return true;
}

bool Game_loadGame(struct Game* game, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return false;
    
    for (int i = 0; i < 10; i++) {
        // (void)!消除检查返回值警告
        (void)!fread(game->chessboard->board[i], sizeof(enum PieceType), 9, file);
    }
    (void)!fread(&game->currentPlayer, sizeof(enum Player), 1, file);
    
    int count;
    (void)!fread(&count, sizeof(int), 1, file);
    game->history.count = 0;
    game->history.capacity = count > 0 ? count : 10;
    game->history.moves = realloc(game->history.moves, game->history.capacity * sizeof(struct Move));
    (void)!fread(game->history.moves, sizeof(struct Move), count, file);
    game->history.count = count;
    
    game->gameOver = false;
    fclose(file);
    return true;
}
struct Position findGeneral(struct Chessboard* cb, enum Player player) {
    struct Position pos = {-1, -1};
    enum PieceType targetGeneral = (player == RED) ? R_GENERAL : B_GENERAL;
    int startRow, endRow; // 限定九宫格的行范围
    int startCol = 3, endCol = 5; // 九宫格列范围固定为3-5

    // 根据玩家确定九宫格的行范围
    if (player == RED) {
        startRow = 7;
        endRow = 9;
    } else {
        startRow = 0;
        endRow = 2;
    }
    // 遍历九宫格
    for (int r = startRow; r <= endRow; r++) {
        for (int c = startCol; c <= endCol; c++) {
            if (cb->board[r][c] == targetGeneral) {
                pos.row = r;
                pos.col = c;
                return pos;
            }
        }
    }
    return pos;
}
//应将的核心实现，采用试移动法
//检查一个移动是否能解除当前被将军的状态
//返回：0：移动后仍然被将军，1：移动后不再被将军
bool doesMoveResolveCheck(struct Chessboard* cb, struct Move move, enum Player player) {
    enum PieceType movingPiece = cb->board[move.from.row][move.from.col];
    enum PieceType capturedPiece = cb->board[move.to.row][move.to.col];
    
    cb->board[move.to.row][move.to.col] = movingPiece;
    cb->board[move.from.row][move.from.col] = EMPTY;
    
    bool stillInCheck = Chessboard_isInCheck(cb, player);
    
    cb->board[move.from.row][move.from.col] = movingPiece;
    cb->board[move.to.row][move.to.col] = capturedPiece;
    
    return !stillInCheck;
}
// 用于检查一个移动是否会让自己陷入被将军的状态，避免自杀性移动，采用试移动法
bool willMoveCauseCheck(struct Chessboard* cb, struct Move move, enum Player player) {
    enum PieceType movingPiece = cb->board[move.from.row][move.from.col];
    enum PieceType capturedPiece = cb->board[move.to.row][move.to.col];
    
    cb->board[move.to.row][move.to.col] = movingPiece;
    cb->board[move.from.row][move.from.col] = EMPTY;
    
    bool inCheckAfterMove = Chessboard_isInCheck(cb, player);
    
    cb->board[move.from.row][move.from.col] = movingPiece;
    cb->board[move.to.row][move.to.col] = capturedPiece;
    
    return inCheckAfterMove;
}
// 命令行逻辑保留但不使用
bool parseMoveInput(const char* input, struct Move* move) {
    // 格式: "r1,c1 r2,c2"
    int r1, c1, r2, c2;
    if (sscanf(input, "%d,%d %d,%d", &r1, &c1, &r2, &c2) == 4) {
        move->from.row = r1;
        move->from.col = c1;
        move->to.row = r2;
        move->to.col = c2;
        return true;
    }
    return false;
}
void Chessboard_showPossibleMoves(struct Chessboard* cb, struct Position pos, enum Player player) {
    enum PieceType piece = Chessboard_getPiece(cb, pos);
    if (piece == EMPTY) return;
    if ((player == RED && !isRed(piece)) || (player == BLACK && !isBlack(piece))) return;
    
    printf("可能的走法: ");
    bool hasMove = false;
    
    for (int r = 0; r < 10; r++) {
        for (int c = 0; c < 9; c++) {
            // struct Move move = {pos, {r, c}, EMPTY};
            struct Move move = {0};//消除警告
            move.from = pos;
            move.to.row = r;  
            move.to.col = c;
            move.captured = EMPTY;
            if (Chessboard_isValidMove(cb, move, player)) {
                printf("(%d,%d) ", r, c);
                hasMove = true;
            }
        }
    }
    
    if (!hasMove) {
        printf("无有效走法");
    }
    printf("\n");
}
// 检查玩家是否还有有效走法（用于判断将死）
bool hasAnyValidMoves(struct Chessboard* cb, enum Player player) {
    // 遍历棋盘上所有己方棋子
    for (int r1 = 0; r1 < 10; r1++) {
        for (int c1 = 0; c1 < 9; c1++) {
            struct Position from = {r1, c1};
            enum PieceType piece = Chessboard_getPiece(cb, from);
            
            if ((player == RED && !isRed(piece)) || (player == BLACK && !isBlack(piece))) {
                continue;
            }
            
            // 检查该棋子是否有任何有效走法
            for (int r2 = 0; r2 < 10; r2++) {
                for (int c2 = 0; c2 < 9; c2++) {
                    // struct Move move = {from, {r2, c2}, EMPTY};
                    struct Move move = {0};
                    move.from = from;
                    move.to.row = r2; 
                    move.to.col = c2;
                    move.captured = EMPTY;
                    // printf("尝试移动: (%d,%d) -> (%d,%d)\n", r1, c1, r2, c2);
                    // 这里应该有问题，因为将军时会有大量打印，可以考虑优化，暂时只是新增一个静默版本的函数
                    // if (Chessboard_isValidMove(cb, move, player)) {
                    if (Chessboard_isValidMoveQuiet(cb, move, player)) {
                        return true;  // 找到有效走法
                    }
                }
            }
        }
    }
    return false;  // 没有任何有效走法
}

// 辅助函数
bool isRed(enum PieceType piece) {
    return piece >= R_GENERAL && piece <= R_SOLDIER;
}

bool isBlack(enum PieceType piece) {
    return piece >= B_GENERAL && piece <= B_SOLDIER;
}
bool isSameColor(enum PieceType a, enum PieceType b) {
    if (a == EMPTY || b == EMPTY) return false;
    return (isRed(a) && isRed(b)) || (isBlack(a) && isBlack(b));
}
bool isInPalace(struct Position pos, enum Player player) {
    if (pos.col < 3 || pos.col > 5) return false;
    if (player == RED) return pos.row >= 7 && pos.row <= 9;
    else return pos.row >= 0 && pos.row <= 2;
}
bool isOwnPiece(enum Player player, enum PieceType piece) {
    if (piece == EMPTY) return false; 
    return (player == RED && isRed(piece)) || (player == BLACK && isBlack(piece));
}
bool isGeneral(enum PieceType piece) {
    return piece == R_GENERAL || piece == B_GENERAL;
}
bool isValidPiece(enum PieceType piece){
    return piece >= EMPTY && piece <= B_SOLDIER;
}
// 从 from 到 to 的直线路径上，除了起点和终点外，中间是否全部为空，应该不包含终点
bool isStraightPathClear(struct Chessboard* cb, struct Position from, struct Position to) {
    if (from.row != to.row && from.col != to.col) return false;//斜线移动
    //水平移动 (from.row == to.row)：只改变列坐标
    // 向右移动：stepCol = 1
    // 向左移动：stepCol = -1
    // 垂直移动 (from.row != to.row)：只改变行坐标
    // 向下移动：stepRow = 1
    // 向上移动：stepRow = -1
    // int stepRow = 0, stepCol = 0;
    // if (from.row == to.row) stepCol = (to.col > from.col) ? 1 : -1;
    // else stepRow = (to.row > from.row) ? 1 : -1;
    int stepRow = (to.row > from.row) - (to.row < from.row); // -1, 0, +1
    int stepCol = (to.col > from.col) - (to.col < from.col);
    // 从起始位置开始，按计算好的方向逐步移动
    // 每移动一格就检查该位置是否为空
    // 如果遇到非空位置（有棋子），说明路径被阻挡，返回false
    // 当到达目标位置时停止检查
    for (int r = from.row + stepRow, c = from.col + stepCol; !(r == to.row && c == to.col);r += stepRow, c += stepCol) {
        if (cb->board[r][c] != EMPTY) return false;
    }
    return true;
}
/* 暂未用到 可能留到以后拓展使用 */
/**
 * @brief 获取当前选中棋子的信息（含合法性校验）
 * @param cb 棋盘指针
 * @param selectedPos 选中的坐标
 * @param currentPlayer 当前行棋玩家（用于判断是否为己方棋子）
 * @return 选中棋子的信息结构体（isValid为true表示有效）
 */
 struct SelectedPieceInfo getSelectedPieceInfo(struct Chessboard* cb, struct Position selectedPos, enum Player currentPlayer) {
    struct SelectedPieceInfo info = {
        .isValid = false,
        .type = EMPTY,
        .pos = {-1, -1},
        .owner = currentPlayer  // 无效时默认当前玩家，不影响判断
    };

    // 校验坐标是否在棋盘范围内
    if (selectedPos.row < 0 || selectedPos.row >= 10 || 
        selectedPos.col < 0 || selectedPos.col >= 9) {
        return info;  // 坐标非法，返回无效信息
    }

    // 读取选中位置的棋子类型
    enum PieceType piece = Chessboard_getPieceAt(cb, selectedPos.row, selectedPos.col);
    if (piece == EMPTY) {
        return info;  // 无棋子，返回无效信息
    }

    // 校验棋子是否属于当前玩家
    enum Player pieceOwner = isRed(piece) ? RED : BLACK;
    if (pieceOwner != currentPlayer) {
        return info;  // 非己方棋子，返回无效信息
    }

    // 所有校验通过，填充有效信息
    info.isValid = true;
    info.type = piece;
    info.pos = selectedPos;
    info.owner = pieceOwner;
    return info;
}
/*
*********************************用于支持终端运行*****************************************
*/
void GameRun_InTerminal(struct Game* game) {
    char input[50];
    struct Move move;
    
    while (!game->gameOver) {
        ChessboardDisplay_InTerminal(game->chessboard);
        
        // 显示是否被将军
        if (Chessboard_isInCheck(game->chessboard, game->currentPlayer)) {
            printf("\n警告: %s方被将军了！必须解除将军状态！\n", 
                   (game->currentPlayer == RED) ? "红" : "黑");
        }
        
        printf("\n%s方行棋 (红方: %d, 黑方: %d)\n", 
               (game->currentPlayer == RED) ? "红" : "黑", RED, BLACK);
        printf("棋子说明: 红方(R): 将帅(K)、士(A)、相(E)、马(H)、车(C)、炮(N)、兵(S)\n");
        printf("          黑方(B): 将帅(k)、士(a)、象(e)、马(h)、车(c)、炮(n)、卒(s)\n");
        printf("命令: \n");
        printf("  移动棋子: 输入起始位置和目标位置，例如 '9,4 8,4'\n");
        printf("  走法提示: 输入 'hint 行,列' 查看指定位置棋子的可能走法\n");
        printf("  悔棋: 输入 'undo'\n");
        printf("  保存游戏: 输入 'save 文件名'\n");
        printf("  加载游戏: 输入 'load 文件名'\n");
        printf("  退出: 输入 'exit'\n");
        printf("请输入命令: ");
        
        if (!fgets(input, sizeof(input), stdin)) {
            return; // 或者 break/continue
        }
        input[strcspn(input, "\n")] = '\0';  // 移除换行符
        
        if (strcmp(input, "exit") == 0) {
            game->gameOver = true;
            break;
        } else if (strcmp(input, "undo") == 0) {
            if (game->history.count >= 2) {  // 悔棋两步，回到自己上一轮
                Chessboard_undoMove(game->chessboard, &game->history);
                Chessboard_undoMove(game->chessboard, &game->history);
            } else {
                printf("无法悔棋，步数不足！\n");
                getchar();  // 等待用户按回车
            }
            continue;
        } else if (strncmp(input, "save ", 5) == 0) {
            const char* filename = input + 5;
            if (Game_saveGame(game, filename)) {
                printf("游戏已保存到 %s\n", filename);
            } else {
                printf("保存失败！\n");
            }
            getchar();  // 等待用户按回车
            continue;
        } else if (strncmp(input, "load ", 5) == 0) {
            const char* filename = input + 5;
            if (Game_loadGame(game, filename)) {
                printf("游戏已从 %s 加载\n", filename);
            } else {
                printf("加载失败！\n");
            }
            getchar();  // 等待用户按回车
            continue;
        } else if (strncmp(input, "hint ", 5) == 0) {
            const char* posStr = input + 5;
            struct Position pos;
            if (sscanf(posStr, "%d,%d", &pos.row, &pos.col) == 2) {
                Chessboard_showPossibleMoves(game->chessboard, pos, game->currentPlayer);
            } else {
                printf("无效的位置格式！请使用 'hint 行,列'\n");
            }
            getchar();  // 等待用户按回车
            continue;
        } else if (parseMoveInput(input, &move)) {
            if (Chessboard_makeMove(game->chessboard, move, &game->history)) {
                // 检查对方是否被将死
                if (Chessboard_isInCheck(game->chessboard, (game->currentPlayer == RED) ? BLACK : RED) &&
                    !hasAnyValidMoves(game->chessboard, (game->currentPlayer == RED) ? BLACK : RED)) {
                    ChessboardDisplay_InTerminal(game->chessboard);
                    printf("%s方胜利！\n", (game->currentPlayer == RED) ? "红" : "黑");
                    game->gameOver = true;
                    break;
                }
                Game_switchPlayer(game);
            } else {
                printf("无效的走法！");
                if (Chessboard_isInCheck(game->chessboard, game->currentPlayer)) {
                    printf(" 您正被将军，请先解除将军状态！");
                }
                printf("\n");
                getchar();  // 等待用户按回车
            }
            continue;
        } else {
            printf("无效的命令！请重试\n");
            getchar();  // 等待用户按回车
            continue;
        }
    }
}
void GameRun_Terminal_main(void)
{ 
#ifdef _WIN32
    // Windows平台设置UTF-8代码页
    system("chcp 65001 > nul");
#else
    // 设置终端为UTF-8编码（Ubuntu系统）
    setenv("LANG", "en_US.UTF-8", 1);
#endif
    struct Game game;
    Game_initialize(&game);
    GameRun_InTerminal(&game);
    
    // 释放内存
    free(game.history.moves);
    free(game.chessboard);
    
    return;
}
void ChessboardDisplay_InTerminal(struct Chessboard* cb) {
#ifdef _WIN32
    system("cls");
#else
    //等价于system("clear");
    fputs("\033[2J\033[H", stdout);
    fflush(stdout);
#endif
    
    printf("  0 1 2 3 4 5 6 7 8\n");
    printf(" +-+-+-+-+-+-+-+-+-+\n");
    
    for (int i = 0; i < 10; i++) {
        printf("%d|", i);
        for (int j = 0; j < 9; j++) {
            printf("%c|", Chessboard_pieceToChar(cb->board[i][j]));
        }
        printf("\n");
        printf(" +-+-+-+-+-+-+-+-+-+\n");
    }
}
/*
*********************************用于支持终端运行*****************************************
*/