# First target is the default if you just say "make"
all: dots dots2

UNAME := $(shell uname)

ifneq ($(UNAME), Linux)
$(error This is only meant to be compiled on Linux)
endif

dots: dots.c remote_gfx.c remote_gfx.h
	gcc -g -Wall -Werror=vla -Werror -Wno-error=unused-function \
          -Wno-error=unused-variable -o dots \
          dots.c remote_gfx.c \
          -lvncserver -lm -lz -g

dots2: dots2.c remote_gfx.c remote_gfx.h
	gcc -g -Wall -Werror=vla -Werror -Wno-error=unused-function \
          -Wno-error=unused-variable -o dots2 \
          dots2.c remote_gfx.c \
          -lvncserver -lm -lz -g

test_dots1: dots.c tester.c
	gcc -g -Wall -Werror=vla -Werror -Wno-error=unused-function \
          -Wno-error=unused-variable -o test_dots1 \
          -Wl,--wrap=main dots.c -DDOTS1 tester.c \
          -lm -g

test_dots2: dots2.c tester.c
	gcc -g -Wall -Werror=vla -Werror -Wno-error=unused-function \
          -Wno-error=unused-variable -o test_dots2 \
          -Wl,--wrap=main dots2.c -DDOTS2 tester.c \
          -lm -g

# Only runs up until the first test that fails
.PHONY: tests
tests: all test_dots1 test_dots2
	python3 -m unittest --verbose --failfast tests.py

# Always tries to run all tests
.PHONY: all-tests
all-tests: all
	python3 -m unittest --verbose test.py

.PHONY: tests1
tests1: test_dots1
	python3 tests.py --verbose --failfast --parts=1

.PHONY: tests2
tests2: test_dots1
	python3 tests.py --verbose --failfast --parts=2

.PHONY: tests3
tests3: test_dots1
	python3 tests.py --verbose --failfast --parts=3

.PHONY: tests4
tests4: test_dots2
	python3 tests.py --verbose --failfast --parts=4

.PHONY: tests5
tests5: test_dots2
	python3 tests.py --verbose --failfast --parts=5

.PHONY: tests6
tests6: test_dots2
	python3 tests.py --verbose --failfast --parts=6

.PHONY: tests7
tests7: test_dots2
	python3 tests.py --verbose --failfast --parts=7

grade: test_dots1 test_dots2
	python3 tests.py --gradescope --skip-after-fail

submission: all
	@echo -n "# " > diffs.diff
	@echo -n $(shell whoami) >> diffs.diff
	@echo -n " (" >> diffs.diff
	@echo -n "$(shell getent passwd ${USER} | cut -d: -f 5 | cut -d, -f 1)" >> diffs.diff
	@echo ")" >> diffs.diff
	@git diff dots.c >> diffs.diff
	@echo "-----------------------------------" >> diffs.diff
	@git diff --no-index dots.c dots2.c >> diffs.diff || true
	@zip submission.zip diffs.diff dots.c dots2.c
	@rm diffs.diff
	@echo submission.zip is ready.

.PHONY: clean
clean:
	@rm -f dots dots2 tester diffs.diff submission.zip test_dots1 test_dots2
	@rm -rf __pycache__
