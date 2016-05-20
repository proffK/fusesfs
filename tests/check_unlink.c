#include <check.h>
#include <bdev/filedev.h>
#include <sfs/unit.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>


#define TRUE_IMAGE "trueimage512"
#define TRUE_SIZE (256 * 40)
#define TI_TIME 0
#define TI_ENTRY_START 0x1400
#define TI_DEL_BEGIN 0x1440
#define TI_VOL_IDENT 0x27C0

START_TEST(test_sfs_unlink)
        filedev_data fdev;
        blockdev bdev;
        sfs_unit fs;
        char teststr[] = "nodir/testfile";

        fdev.fd = -1;
        
        filedev_create(&bdev, &fdev, 512, TRUE_SIZE);

        fdev.filename = "testfile";
        blockdev_init(&bdev);

        sfs_init(&fs, &bdev);
        
        ck_assert_int_eq(fs.bdev, &bdev);        
        ck_assert_int_eq(fs.time, TI_TIME);
        ck_assert_int_eq(fs.entry_start, TI_ENTRY_START);
        ck_assert_int_eq(fs.del_begin, TI_DEL_BEGIN);
        ck_assert_int_eq(fs.vol_ident, TI_VOL_IDENT);

        ck_assert(sfs_creat(&fs, "testfile") == 0);
        ck_assert(sfs_unlink(&fs, "testfile") == 0);
        ck_assert(sfs_unlink(&fs, "testfile") == -1);
        ck_assert(sfs_unlink(&fs, teststr) == -1);

        bdev.release(&bdev);
        ck_assert(errno == 0);
END_TEST

static int gen_test_file(char* filename, ssize_t size)
{
        int fd = 0;
        uint8_t* buf = (uint8_t*) malloc (size);


        if (buf == NULL)
                exit(EXIT_FAILURE);

        fd = open(TRUE_IMAGE, O_CREAT | O_RDWR, 0600);
        size = read(fd, buf, size);
        close(fd);
        fd = open(filename, O_CREAT | O_RDWR, 0600);

        if (fd == -1)
                exit(EXIT_FAILURE);

        size = write(fd, buf, size);

        if (size == -1)
                exit(EXIT_FAILURE);

        close(fd);

        free(buf);

        return size;
}
static Suite* init_suite(void)
{
        Suite* s = NULL;
        TCase* tc_init = NULL;

        s = suite_create("SFS init");

        /* Core test case */
        tc_init = tcase_create("Init");

        tcase_add_test(tc_init, test_sfs_unlink);

        suite_add_tcase(s, tc_init);

        return s;
}

int main(void) {
        int number_failed = 0;
        Suite* sfs_init_suite = NULL;
        SRunner* sr = NULL;

        sfs_init_suite = init_suite();
        sr = srunner_create(sfs_init_suite);

        gen_test_file("testfile", TRUE_SIZE);

        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);
        srunner_free(sr);
        
        unlink("testfile");

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
