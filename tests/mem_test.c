#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include "mem.h"

#define FIFO_SIZE 128

static int setup(void **state)
{
    *state = fifo_init(FIFO_SIZE);
    assert_non_null(*state);
    return 0;
}

static int teardown(void **state)
{
    fifo_free(*state);
    return 0;
}

static void fifo_init_test(void **state)
{
    struct fifo_buf *buf = fifo_init(512);

    assert_non_null(buf);
    assert_int_equal(fifo_curr_pos(buf), 0);

    fifo_free(buf);
}

static void fifo_reset_test(void **state)
{
    struct fifo_buf *buf = *state;
    char tmp[16];

    fifo_write(buf, "123", 4);
    fifo_read(buf, tmp, sizeof tmp);
    fifo_reset(buf);

    assert_int_equal(fifo_curr_pos(buf), 0);
    assert_int_equal(fifo_min_pos(buf), 0);
    assert_int_equal(fifo_max_pos(buf), 0);
}

static void fifo_available_data__should_return_num_avaiable_bytes(void **state)
{
    struct fifo_buf *buf = *state;
    char tmp[8];

    fifo_write(buf, "123", 4);
    assert_int_equal(fifo_available_data(buf), 4);
    fifo_read(buf, tmp, 2);
    assert_int_equal(fifo_available_data(buf), 2);
}

static void fifo_write__should_wrap_around_when_hitting_end_of_mem_area(void **state)
{
    struct fifo_buf *buf = *state;
    char tmp[FIFO_SIZE * 2], tmp2[FIFO_SIZE];

    memset(tmp, 'A', FIFO_SIZE);
    memset(tmp + FIFO_SIZE, 'B', FIFO_SIZE);

    assert_int_equal(fifo_write(buf, tmp, sizeof(tmp)), sizeof(tmp));
    assert_int_equal(fifo_available_data(buf), FIFO_SIZE);
    assert_int_equal(fifo_curr_pos(buf), FIFO_SIZE);
    assert_int_equal(fifo_min_pos(buf), FIFO_SIZE);
    assert_int_equal(fifo_max_pos(buf), sizeof(tmp));

    assert_int_equal(fifo_read(buf, tmp2, sizeof(tmp2)), FIFO_SIZE);
    assert_memory_equal(tmp + FIFO_SIZE, tmp2, FIFO_SIZE);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(fifo_init_test),
        cmocka_unit_test_setup_teardown(fifo_reset_test, setup, teardown),
        cmocka_unit_test_setup_teardown(
            fifo_available_data__should_return_num_avaiable_bytes, setup, teardown),
        cmocka_unit_test_setup_teardown(
            fifo_write__should_wrap_around_when_hitting_end_of_mem_area, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
