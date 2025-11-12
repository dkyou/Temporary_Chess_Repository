#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CHESS_OK = 0,
    CHESS_ERR = -1,
    CHESS_EINVAL = -2,
    CHESS_EIO = -3,
} chess_err_t;

#ifdef __cplusplus
}
#endif
