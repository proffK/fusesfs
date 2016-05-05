#define  UTILS_NON_CONFLICT
#include <sfs/utils.h>
#include <check.h>

#include <errno.h>
#include <stdio.h>

#define NUM_OF_WRONG_SYM 70

START_TEST(test_utils)
        int i = 0;
        int l = 0;
        char test_str1[1] = { '\0' } ;
 //       char test_str2[6];
 //       char test_str3[4];

        char wrong_chars[NUM_OF_WRONG_SYM] = {
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
                0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
                0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,
                0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x80, 0x81, 0x82,
                0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
                0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
                0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, '"' , '*' ,
                ':' , '<' , '>' , '?' , '\\', 0x5C, 0x7F, 0xA0 
        };
         
        for (i = 0; i < NUM_OF_WRONG_SYM; i++) 
                ck_assert_int_eq(is_correct_char(wrong_chars[i]), -1);

        char wrong_string[18] = "F*ck you, NVIDIA!";

        ck_assert_int_eq(is_correct_string(wrong_string, 18), -1);

        ck_assert_int_eq(strlen("alena"), 5);
        l = strlen(test_str1);
        ck_assert_int_eq(l, 0);

        ck_assert_int_eq(strnlen("", 0), 0);
        ck_assert_int_eq(strnlen("", 3), 0);
        ck_assert_int_eq(strnlen("alena", 0), 0);
        ck_assert_int_eq(strnlen("alena", 4), 4);
        ck_assert_int_eq(strnlen("alena", 7), 5);
        ck_assert_int_eq(strnlen("alena", 6), 5);

        ck_assert_int_eq(strcmp("alena", "alena"), 0);
        ck_assert_int_eq(strcmp("alena", "blena"), -1);
        ck_assert_int_eq(strcmp("blena", "alena"), 1);

        ck_assert_int_eq(strncmp("alena", "alenb", 4), 0);
        ck_assert_int_eq(strncmp("alena", "alenb", 5), -1);
        ck_assert_int_eq(strncmp("alena", "alena", 5), 0);
        ck_assert_int_eq(strncmp("alena", "alena", 6), 0);
        ck_assert_int_eq(strncmp("alena", "alena", 7), 0);

//        ck_assert_int_eq(strcpy(test_str2, "alena"), test_str2);
//        ck_assert_int_eq(strcmp(test_str2, "alena"), 0);
//        ck_assert_int_eq(strncpy(test_str3, "alena", 4), test_str3);
//        ck_assert_int_eq(strcmp(test_str3, "ale"), 0);
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

        return (number_failed == 0) ? 0 : -1;
}
