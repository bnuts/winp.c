#include "pipes.h"


int pipe_open(
    HANDLE* read_pipe, int inherit_read,
    HANDLE* write_pipe, int inherit_write)
{
    if(!read_pipe || !write_pipe) return 0;
    if(!CreatePipe(read_pipe, write_pipe, NULL, 0)) return 0;

    BOOL result = TRUE;
    if(inherit_read) {
        result = SetHandleInformation(
            *read_pipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    }
    if(result && inherit_write) {
        result = SetHandleInformation(
            *write_pipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    }
    if(result) {
        return 1;
    } else {
        pipe_close(read_pipe);
        pipe_close(write_pipe);
        return 0;
    }
}


pipes_t* pipes_new()
{
    pipes_t* pipes = calloc(1, sizeof(pipes_t));
    if(!pipes) return NULL;

    for(;;) {
        if(!pipe_open(&pipes->stdin_read, 1, &pipes->stdin_write, 0)) break;
        if(!pipe_open(&pipes->stdout_read, 0, &pipes->stdout_write, 1)) break;
        if(!pipe_open(&pipes->stderr_read, 0, &pipes->stderr_write, 1)) break;
        return pipes;
    }
    pipes_free(pipes);
    return NULL;
}


void pipe_close(HANDLE* pipe)
{
    if(!pipe || *pipe == INVALID_HANDLE_VALUE) return;
    if(*pipe) CloseHandle(*pipe);
    *pipe = INVALID_HANDLE_VALUE;
}


void pipes_free(pipes_t* pipes)
{
    if(!pipes) return;
    pipe_close(&pipes->stdin_read); pipe_close(&pipes->stdin_write);
    pipe_close(&pipes->stdout_read); pipe_close(&pipes->stdout_write);
    pipe_close(&pipes->stderr_read); pipe_close(&pipes->stderr_write);
    free(pipes);
}
