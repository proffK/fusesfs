#include <check.h>
#include <bdev/filedev.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#define TESTFILE_NAME "testdev"
#define TESTFILE_SIZE (512 * 10)
#define TESTFILE_PERM 0600

START_TEST(test_filedev_create)
        filedev_data fdev;
        blockdev bdev;
        
        ck_assert(filedev_create(&bdev, NULL, 512, 512 * 10) == &bdev); 
        ck_assert(filedev_create(NULL, &fdev, 512, 512 * 10) == NULL); 
        ck_assert(filedev_create(&bdev, &fdev, 512, 513 * 10) == NULL); 
END_TEST

START_TEST(test_filedev_init)
        filedev_data fdev;
        blockdev bdev;

        fdev.fd = -1;
        
        ck_assert(filedev_create(&bdev, &fdev, 512, 512 * 10) == &bdev); 

        ck_assert_int_eq(blockdev_init(NULL), -1);
        
        fdev.filename = NULL;
        ck_assert_int_eq(blockdev_init(&bdev), -1);
        ck_assert(errno == EINVAL);

        fdev.filename = TESTFILE_NAME;
        ck_assert_int_eq(blockdev_init(&bdev), 0);
        ck_assert(errno == 0);
END_TEST
 


static int gen_test_file(char* filename, ssize_t size)
{
        int fd = 0;
        uint8_t* buf = (uint8_t*) malloc (size);

        if (buf == NULL)
                exit(EXIT_FAILURE);

        fd = open(filename, O_CREAT | O_RDWR, TESTFILE_PERM);

        if (fd == -1)
                exit(EXIT_FAILURE);

        size = write(fd, buf, size);

        if (size == -1)
                exit(EXIT_FAILURE);

        perror("!!!");
        close(fd);

        return size;
}

static Suite* blockdev_suite(void)
{
        Suite* s = NULL;
        TCase* tc_init = NULL;
        TCase* tc_read = NULL;
        TCase* tc_write = NULL;

        s = suite_create("Block device layer");

        /* Core test case */
        tc_init = tcase_create("Init");

        tcase_add_test(tc_init, test_filedev_create);
        tcase_add_test(tc_init, test_filedev_init);

        suite_add_tcase(s, tc_init);

        return s;
}

int main(void) {
        int number_failed = 0;
        Suite* bdev_suite = NULL;
        SRunner* sr = NULL;

        bdev_suite = blockdev_suite();
        sr = srunner_create(bdev_suite);

        gen_test_file(TESTFILE_NAME, TESTFILE_SIZE);

        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);
        srunner_free(sr);

        unlink(TESTFILE_NAME);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
