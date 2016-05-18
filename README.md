winp.c
===

Read outputs of the process for Windows

## Usage

```c
winp_t winp = {0};
char input[] = "foobar";
size_t input_len = strlen(input);

winp_run(&winp, "cat", input, input_len);
printf("stdout: %s\n", winp.output);
printf("stderr: %s\n", winp.error);

winp_free(&winp);
```
