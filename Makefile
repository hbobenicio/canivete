SRC = \
	$(wildcard src/canivete/*.c) \
	$(wildcard src/canivete/utils/*.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)
BIN = canivete
BIN_STATIC = $(BIN)-static

# compiler and linker flags
# _DEFAULT_SOURCE: needed by glibc to define struct dirent d_type macros
CFLAGS  = -Wall -Wextra -pedantic -std=c17 -MMD -I ./src/ -I ./vendor/stb/0.67 -D_DEFAULT_SOURCE
LDFLAGS = -Wall -Wextra -pedantic -std=c17
LDLIBS  = -lssl -lcrypto -lsqlite3

# options for static builds with docker
CONTAINER_NAME = canivete
IMG_NAME = canivete/alpine

.PHONY: all release release-static debug distclean clean

all: debug

release: CFLAGS += -O3
release: LDFLAGS += -O3
release: $(BIN)
	strip $(BIN)

release-static: CFLAGS += -static
release-static: LDFLAGS += -static
release-static: $(BIN_STATIC)

ifeq ($(CC),clang)
debug: CFLAGS += -fsanitize=address
debug: LDFLAGS += -fsanitize=address
endif

debug: CFLAGS += -g -O0 -DDEBUG
debug: LDFLAGS += -g -O0
debug: $(BIN)

# distclean is used here just to prevent copying locally compiled objects from host to alpine builder container
# TODO check if failures stop the execution
$(BIN_STATIC): $(SRC) Makefile distclean
	docker build -f docker/alpine/Dockerfile \
		--build-arg CFLAGS="$(CFLAGS)" \
		--build-arg LDFLAGS="$(LDFLAGS)" \
		-t $(IMG_NAME) .
	docker container create --name $(CONTAINER_NAME) $(IMG_NAME)
	docker cp $(CONTAINER_NAME):/app/$(BIN) ./$@
	docker container rm $(CONTAINER_NAME)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEP)

distclean: clean
	rm -f $(BIN) $(BIN_STATIC)

clean:
	find . \
		-type f -name '*.d' -delete -o \
		-type f -name '*.o' -delete
	# docker container rm -f $(CONTAINER_NAME)
	# docker image rm -f $(IMG_NAME)

