#include <check.h>
#include <bdev/filedev.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define TESTFILE_NAME "testfile"
#define TRUE_NAME1     "trueimage512"
#define TRUE_NAME2     "trueimage256"
#define TRUE_NAME3     "trueimage1024"
#define TESTFILE_SIZE (512 * 20)
#define TESTFILE_PERM 0600

#define TIMESTAMP1 404
#define TIMESTAMP2 0x27c4
#define CHECKSUM 448

static int clear_timestamp(char* filename) 
{
        int fd = 0;
        uint64_t new_time = 0;

        fd = open(filename, O_RDWR);
        lseek(fd, TIMESTAMP1, SEEK_SET);
        if (write(fd, &new_time, sizeof(uint64_t)) == -1)
                return -1;

        lseek(fd, TIMESTAMP2, SEEK_SET);
        if (write(fd, &new_time, sizeof(uint64_t)) == -1)
                return -1;

        lseek(fd, CHECKSUM, SEEK_SET);
        if (write(fd, &new_time, sizeof(uint16_t)) == -1)
                return -1;

        close(fd);
        
        return 0;
}

static int gen_test_file(char* filename, ssize_t size)
{
        int fd = 0;
        uint8_t* buf = (uint8_t*) calloc (size, sizeof(uint8_t));

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

START_TEST(test_mkfs)
        pid_t pid = 0;
        int ret = 0;
        
        clear_timestamp(TRUE_NAME1);
        clear_timestamp(TRUE_NAME2);
        clear_timestamp(TRUE_NAME3);

        pid = fork();
        if (pid == 0) {
                execlp("../src/mksfs/mksfs", "mksfs", "-b", "512", TESTFILE_NAME, NULL);
        }
        wait(&ret);
        ck_assert_int_eq(ret, 0);

        clear_timestamp(TESTFILE_NAME);

        pid = fork();
        if (pid == 0) {
                execlp("cmp", "cmp", "-l", TESTFILE_NAME, TRUE_NAME1, NULL);
        }
        wait(&ret);
        ck_assert_int_eq(ret, 0);

        unlink(TESTFILE_NAME);
        gen_test_file(TESTFILE_NAME, TESTFILE_SIZE);

        pid = fork();
        if (pid == 0) {
                execlp("../src/mksfs/mksfs", "mksfs", "-b", "256", TESTFILE_NAME, NULL);
        }
        wait(&ret);
        ck_assert_int_eq(ret, 0);

        clear_timestamp(TESTFILE_NAME);

        pid = fork();
        if (pid == 0) {
                execlp("cmp", "cmp", "-l", TESTFILE_NAME, TRUE_NAME2, NULL);
        }
        wait(&ret);
        ck_assert_int_eq(ret, 0);

        unlink(TESTFILE_NAME);
        gen_test_file(TESTFILE_NAME, TESTFILE_SIZE);

        pid = fork();
        if (pid == 0) {
                execlp("../src/mksfs/mksfs", "mksfs", "-b", "1024", TESTFILE_NAME, NULL);
        }
        wait(&ret);
        ck_assert_int_eq(ret, 0);

        clear_timestamp(TESTFILE_NAME);

        pid = fork();
        if (pid == 0) {
                execlp("cmp", "cmp", "-l", TESTFILE_NAME, TRUE_NAME3, NULL);
        }
        wait(&ret);
        ck_assert_int_eq(ret, 0);
END_TEST

static Suite* blockdev_suite(void)
{
        Suite* s = NULL;
        TCase* tc_init = NULL;

        s = suite_create("Make filesystem");

        /* Core test case */
        tc_init = tcase_create("Mkfs");

        tcase_add_test(tc_init, test_mkfs);

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
