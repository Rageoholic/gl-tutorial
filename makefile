COMPONENTS = gl-basic-rendering-ebo gl-basic-rendering gl-vertex-shader-color $(SHADER_TESTS)

WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-variable	\
-Wno-error=unused-parameter -Wno-missing-field-initializers -Wno-unused-function
CFLAGS = $(WARNINGS) --std=c99 -MD -MP -masm=intel $(OPTFLAGS)
CCFLAGS = $(WARNINGS) --std=c++17 -MD -MP -masm=intel $(OPTFLAGS)
DEPS = $(shell find . -name "*.d")
_SHADERS = $(shell find . -name "*.vert")  $(shell find . -name "*.frag")
SHADER_TESTS = $(patsubst %, %.test, $(_SHADERS))

LIBS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
LDFLAGS = $(LIBS)

ifndef mode
	mode=debugopt
endif

ifeq ($(mode),release)
	OPTFLAGS += -flto -O2 -g
	LDFLAGS += -flto -g
endif
ifeq ($(mode),debug)
	OPTFLAGS += -g -O0
endif
ifeq ($(mode),debugopt)
	OPTFLAGS += -O2 -g
endif


all: $(COMPONENTS)


-include $(DEPS)

-include .user.mk		#User defined includes. Use this to configure specific settings on build files
RUTILS_DIR = rutils/
-include rutils/rutils.mk

gl-basic-rendering-ebo: rgl.o gl-basic-rendering-ebo.o  rutils.a glad.o
gl-vertex-shader-color: rgl.o gl-vertex-shader-color.o  rutils.a glad.o
gl-basic-rendering: rgl.o gl-basic-rendering.o  rutils.a glad.o

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CCFLAGS)


%.o: %.c
	sparse $< -Wsparse-error -Wsparse-all -Wno-declaration-after-statement -Wno-decl
	$(CC) -c $< -o $@ $(CFLAGS)

%.vert.test: %.vert
	glslangValidator $<
	date > $@

%.frag.test: %.frag
	glslangValidator $<
	date > $@

%.o: %.asm
	$(AS) -felf32 $< -o $@

clean:
	-$(RM)  `find . -name "*.o"` ` find . -name "*.d"` $(COMPONENTS) rageOS.iso

.PHONY: all clean info debug debug-opt release default
