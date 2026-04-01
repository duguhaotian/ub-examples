/*
 * test_runner.c - Test runner for obmmctl
 */

#include <stdio.h>

/* Forward declarations */
extern int test_cli_parser(void);
extern int run_sysfs_reader_tests(void);
extern int run_libobmm_wrap_tests(void);

int main(void)
{
    int failed = 0;

    printf("=== Running obmmctl tests ===\n\n");

    failed += test_cli_parser();
    failed += run_sysfs_reader_tests();
    failed += run_libobmm_wrap_tests();

    printf("\n=== Summary ===\n");
    if (failed == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("%d test modules failed\n", failed);
        return 1;
    }
}