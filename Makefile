PROGRAMS := gzmstat gzmcat

CC := gcc
CFLAGS := -Wall -pedantic -MMD -I. -Isrc -ffunction-sections -fdata-sections
OPTFLAGS := -O3
AR := ar
CP := cp

.PHONY: all clean
.DEFAULT_GOAL := all

all: $(PROGRAMS)

clean:
	$(RM) -r build $(PROGRAMS)

#   libgzx
LIBGZX_C_FILES := $(shell find src/libgzx -type f -name *.c)
LIBGZX_O_FILES := $(foreach f,$(LIBGZX_C_FILES:.c=.o),build/$f)
LIBGZX_DEP_FILES := $(LIBGZX_O_FILES:.o=.d)

$(shell mkdir -p build $(foreach dir,$(shell find src/libgzx -type d),build/$(dir)))

build/libgzx.a: $(LIBGZX_O_FILES)
	$(AR) rcs $@ $^

build/src/libgzx/%.o: src/libgzx/%.c
	$(CC) -c $(CFLAGS) $(OPTFLAGS) $< -o $@

#   Programs
define COMPILE =
build/$(1): $(shell find src/$1 -type f -name *.c) build/libgzx.a
	$(CC) -Wl,--gc-sections $(CFLAGS) $(OPTFLAGS) $$^ -o $$@

$(1): build/$(1)
	$(CP) $$< $$@
endef
$(foreach p,$(PROGRAMS),$(eval $(call COMPILE,$(p))))

PROGRAM_DEP_FILES := $(foreach prog,$(PROGRAMS),build/$(prog:=.d))

-include $(PROGRAM_DEP_FILES)
-include $(LIBGZX_DEP_FILES)
