#include "assertion-macros/assertion-macros.h"
#include "winp.h"


void cat_command_test1()
{
    winp_t winp = {0};
    char input[] = "foobar";
    size_t input_len = strlen(input);

    assert_equal(winp_run(&winp, "cat", input, input_len), 1);
    assert_equal((int)winp.return_code, 0);
    assert_str_equal(winp.output, input);
    assert_equal(winp.output_len, input_len);
    assert_null(winp.error);
    assert_equal(winp.error_len, 0);

    winp_free(&winp);
}


void cat_command_test2()
{
    winp_t winp = {0};
    char input[] = "f\0o\0o\0b\0a\0r";
    size_t input_len = 11;

    assert_equal(winp_run(&winp, "cat", input, input_len), 1);
    assert_equal((int)winp.return_code, 0);
    assert_equal(memcmp(winp.output, input, input_len), 0);
    assert_equal(winp.output_len, input_len);
    assert_null(winp.error);
    assert_equal(winp.error_len, 0);

    winp_free(&winp);
}


void cat_help_command_test()
{
    winp_t winp = {0};

    assert_equal(winp_run(&winp, "cat --help", NULL, 0), 1);
    assert_equal((int)winp.return_code, 0);
    assert_not_null(winp.output);
    assert_not_equal(winp.output_len, 0);
    assert_null(winp.error);
    assert_equal(winp.error_len, 0);

    winp_free(&winp);
}


void unknown_command_test()
{
    winp_t winp = {0};

    assert_equal(winp_run(&winp, "unknown", NULL, 0), 0);
    assert_equal((int)winp.return_code, 0);
    assert_null(winp.output);
    assert_equal(winp.output_len, 0);
    assert_null(winp.error);
    assert_equal(winp.error_len, 0);

    winp_free(&winp);
}


int main()
{
    cat_command_test1();
    cat_command_test2();
    cat_help_command_test();
    unknown_command_test();

    return assert_failures();
}
