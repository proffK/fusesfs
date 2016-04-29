#include <check.h>
#include <sys/utils.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

START_TEST(test_utils)
        
        //
END_TEST

static Suite* utils_suite(void)
{
        Suite* s = NULL;
        TCase* tc_init = NULL;

        s = suite_create("Utils for SFS");

        /* Core test case */
        tc_init = tcase_create("Utils");

        tcase_add_test(tc_init, test_utils);

        suite_add_tcase(s, tc_init);

        return s;
}       

int main(void) {
        int number_failed = 0;
        Suite* u_suite = NULL;
        SRunner* sr = NULL;

        u_suite = utils_suite();
        sr = srunner_create(u_suite);

        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);
        srunner_free(sr);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
