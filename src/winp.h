#ifndef WINP_C_WINP_H
#define WINP_C_WINP_H

#if defined(UNICODE) || defined(_UNICODE)
#define winp_run winp_run_w
#else
#define winp_run winp_run_a
#endif

struct winp_t {
    char* output;
    size_t output_len;
    char* error;
    size_t error_len;
    unsigned long return_code;
};

typedef struct winp_t winp_t;

int winp_run_a(winp_t* winp, char* command, char* input, size_t input_len);

int winp_run_w(winp_t* winp, wchar_t* command, char* input, size_t input_len);

void winp_free(winp_t* winp);

#endif
