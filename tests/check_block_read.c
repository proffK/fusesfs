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

#define TESTCHAR '8'

#define BUF_SIZE 512

START_TEST(test_filedev_read)
        filedev_data fdev;
        blockdev bdev;
        buf_t buf[BUF_SIZE];

        fdev.fd = -1;
        
        ck_assert(filedev_create(&bdev, &fdev, 512, 512 * 10) == &bdev); 

        fdev.filename = TESTFILE_NAME;
        ck_assert_int_eq(blockdev_init(&bdev), 0);
        ck_assert(errno == 0);

        ck_assert_int_eq(bdev.read(NULL, buf, 512, 0), 0);
        ck_assert(errno == EFAULT);

        ck_assert_int_eq(bdev.read(&bdev, NULL, 512, 0), 0);
        ck_assert(errno == EFAULT);

        ck_assert_int_eq(bdev.read(&bdev, buf, 513, 0), 0);
        ck_assert(errno == EINVAL);

        ck_assert_int_eq(bdev.read(&bdev, buf, 512, 0), 512);
        ck_assert(errno == 0);
        ck_assert(buf[0] == TESTCHAR);

END_TEST

static int gen_test_file(char* filename, ssize_t size)
{
        int fd = 0;
        uint8_t* buf = (uint8_t*) malloc (size);

        memset(buf, TESTCHAR, size / 2);

        if (buf == NULL)
                exit(EXIT_FAILURE);

        fd = open(filename, O_CREAT | O_RDWR, TESTFILE_PERM);

        if (fd == -1)
                exit(EXIT_FAILURE);

        size = write(fd, buf, size);

        if (size == -1)
                exit(EXIT_FAILURE);

        close(fd);

        free(buf);

        return size;
}

static Suite* blockdev_suite(void)
{
        Suite* s = NULL;
        TCase* tc_read = NULL;

        s = suite_create("Block device layer");

        /* Core test case */
        tc_read = tcase_create("Read");

        tcase_add_test(tc_read, test_filedev_read);

        suite_add_tcase(s, tc_read);

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
