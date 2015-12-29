#ifndef WINP_C_PIPES_H
#define WINP_C_PIPES_H


#include <windows.h>


typedef struct {
    HANDLE stdin_read;
    HANDLE stdin_write;
    HANDLE stdout_read;
    HANDLE stdout_write;
    HANDLE stderr_read;
    HANDLE stderr_write;
} pipes_t;


pipes_t* pipes_new();

void pipe_close(HANDLE* pipe);

void pipes_free(pipes_t* pipes);


#endif
