TEST_SRCS = test/main.c test/vec.c test/hmap.c test/region.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_DEPS = $(TEST_OBJS:.o=.d)

# -Wfatal-errors
CFLAGS = -std=gnu17 -Wall -Wextra -Wswitch-enum -Wcast-align \
	 -Wpointer-arith -Wlogical-op -Wredundant-decls	 \
	 -Werror=incompatible-pointer-types -Wconversion -Wno-gnu -g \
	 -lm -DQCC_TESTING -I./src
#-fsanitize=address -fno-omit-frame-pointer \


run_test: $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

-include $(TEST_DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	-rm -f run_test
	-rm -f $(TEST_OBJS)
	-rm -f $(TEST_DEPS)

.PHONY: clean
