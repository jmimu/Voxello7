CFLAGS= -g -O3 -std=c99 -Wall -Wextra -fopenmp -Isrc -rdynamic -DNDEBUG $(OPTFLAGS)
#-Wpadded
LIBS=-lm -lSDL2 -lSDL2_image -pthread $(OPTLIBS)
PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS_TMP=$(patsubst %.c,%.o,$(SOURCES))
OBJECTS=$(OBJECTS_TMP:src/%=build/%)

DEP = $(OBJECTS:%.o=%.d)

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=bin/voxello7

# The Target Build
all: $(TARGET) tests

dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $@

build:
	@mkdir -p build
	@mkdir -p bin

#build/%.o : src/%.c
#	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

# The -MMD flags additionaly creates a .d file with
# the same name as the .o file.
build/%.o : src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@


# The Unit Tests
.PHONY: tests
tests: CFLAGS += $(TARGET)
tests: $(TESTS)
	sh ./tests/runtests.sh

valgrind:
	VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)

# The Cleaner
clean:
	rm -rf build $(OBJECTS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`
	rm -f $(TARGET) 

# The Install
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

# The Checker
BADFUNCS='[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)'
check:
	@echo Files with potentially dangerous functions:
	@egrep $(BADFUNCS) $(SOURCES) || true
