#include <windows.h>
#include "pipes.h"
#include "winp.h"

typedef struct {
    HANDLE process;
    HANDLE read_pipe;
    HANDLE write_pipe;
    char* str;
    size_t str_len;
} stream_data_t;

char* append_str(char* str, size_t str_len, char* astr, size_t astr_len)
{
    if(!astr) return NULL;
    if(!str) str_len = 0;

    char* temp = realloc(str, str_len + astr_len + 1);
    if(temp) {
        str = temp;
        memcpy(str + str_len, astr, astr_len);
        str[str_len + astr_len] = '\0';
    } else {
        free(str);
        str = NULL;
    }
    return str;
}

DWORD WINAPI read_from_stream(void* arg)
{
    stream_data_t* stream_data = arg;
    if(!stream_data) return NULL;

    BOOL result;
    DWORD bytes_read;
    DWORD str_len = 0;
    char bytes[128 * 1024];
    char* str = NULL;

    for(;;) {
        result = ReadFile(
            stream_data->read_pipe, bytes, sizeof(bytes), &bytes_read, NULL);
        if(!result || bytes_read == 0) break;
        str = append_str(str, str_len, bytes, bytes_read);
        if(!str) break;
        str_len += bytes_read;
    }

    stream_data->str = str;
    stream_data->str_len = str ? str_len : 0;

    return 0;
}

DWORD WINAPI write_to_stream(void* arg)
{
    stream_data_t* stream_data = arg;
    if(!stream_data) return NULL;

    DWORD written;
    WriteFile(
        stream_data->write_pipe, stream_data->str, stream_data->str_len,
        &written, NULL);
    pipe_close(&stream_data->write_pipe);

    return 0;
}

int winp_run_impl(
    winp_t* winp, char* command, char* input, size_t input_len, int is_unicode)
{
    if(!winp) return 0;

    pipes_t* pipes = pipes_new();
    if(!pipes) return 0;

    int result;
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = pipes->stdin_read;
    si.hStdOutput = pipes->stdout_write;
    si.hStdError = pipes->stderr_write;

    const size_t threads_len = input ? 3 : 2;
    HANDLE threads[threads_len];
    memset(threads, 0, threads_len * sizeof(HANDLE));

    for(;;) {
        if(is_unicode) {
            result = CreateProcessW(
                NULL, (wchar_t*)command, NULL, NULL, TRUE,
                NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                NULL, NULL, (STARTUPINFOW*)&si, &pi);
        } else {
            result = CreateProcessA(
                NULL, command, NULL, NULL, TRUE,
                NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                NULL, NULL, (STARTUPINFOA*)&si, &pi);
        }
        if(!result) break;

        stream_data_t stdout_data = {
            pi.hProcess, pipes->stdout_read, pipes->stdout_write, NULL, 0
        };
        threads[0] = CreateThread(
            NULL, 0, read_from_stream, &stdout_data, 0, NULL);
        if(!threads[0]) {result = 0; break;}

        stream_data_t stderr_data = {
            pi.hProcess, pipes->stderr_read, pipes->stderr_write, NULL, 0
        };
        threads[1] = CreateThread(
            NULL, 0, read_from_stream, &stderr_data, 0, NULL);
        if(!threads[1]) {result = 0; break;}

        stream_data_t stdin_data = {
            pi.hProcess, pipes->stdin_read, pipes->stdin_write,
            input, input_len
        };
        if(input) {
            threads[2] = CreateThread(
                NULL, 0, write_to_stream, &stdin_data, 0, NULL);
            if(!threads[2]) {result = 0; break;}
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &winp->return_code);

        // ここで閉じないとブロックされ続ける。
        pipe_close(&pipes->stdin_read);
        pipe_close(&pipes->stdout_write);
        pipe_close(&pipes->stderr_write);

        WaitForMultipleObjects(threads_len, threads, TRUE, INFINITE);

        winp->output = stdout_data.str;
        winp->output_len = stdout_data.str_len;
        winp->error = stderr_data.str;
        winp->error_len = stderr_data.str_len;

        result = 1;
        break;
    }

    if(pi.hProcess) CloseHandle(pi.hProcess);
    if(pi.hThread) CloseHandle(pi.hThread);
    for(size_t i = 0; i < threads_len; ++i) {
        if(threads[i]) CloseHandle(threads[i]);
    }
    pipes_free(pipes);

    return result;
}

int winp_run_a(winp_t* winp, char* command, char* input, size_t input_len)
{
    return winp_run_impl(winp, command, input, input_len, 0);
}

int winp_run_w(winp_t* winp, wchar_t* command, char* input, size_t input_len)
{
    return winp_run_impl(winp, (char*)command, input, input_len, 1);
}

void winp_free(winp_t* winp)
{
    if(winp) {
        free(winp->output);
        free(winp->error);
    }
}
