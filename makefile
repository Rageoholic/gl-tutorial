COMPONENTS = gl-basic-rendering-ebo gl-basic-rendering gl-vertex-shader-color	\
gl-uniform-color gl-passed-color gl-texture-basics gl-transform-basics		\
gl-transform-perspective lighting-experiment textured-lighting	\
multiple-lights gl-transform-test $(SHADER_TESTS)

WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-variable	\
-Wno-error=unused-parameter -Wno-missing-field-initializers -Wno-unused-function
CFLAGS = $(WARNINGS) --std=c99 -MD -MP -masm=intel $(OPTFLAGS)
CCFLAGS = $(WARNINGS) --std=c++17 -MD -MP -masm=intel $(OPTFLAGS) -fpermissive
DEPS = $(shell find . -name "*.d")
_SHADERS = $(shell find . -name "*.vert")  $(shell find . -name "*.frag")
SHADER_TESTS = $(patsubst %, %.test, $(_SHADERS))

LIBS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm
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
gl-uniform-color: rgl.o gl-uniform-color.o  rutils.a glad.o
gl-passed-color: rgl.o gl-passed-color.o  rutils.a glad.o
gl-texture-basics: rgl.o gl-texture-basics.o stb_image.o rutils.a glad.o
gl-transform-basics: rgl.o gl-transform-basics.o stb_image.o rutils.a glad.o
gl-transform-perspective: rgl.o gl-transform-perspective.o stb_image.o rutils.a glad.o
gl-transform-test: rgl.o gl-transform-test.o stb_image.o rutils.a glad.o
lighting-experiment: rgl.o lighting-experiment.o stb_image.o rutils.a glad.o
textured-lighting: rgl.o textured-lighting.o stb_image.o rutils.a glad.o
multiple-lights: rgl.o multiple-lights.o stb_image.o rutils.a glad.o

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CCFLAGS)


%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.vert.test: %.vert
	glslangValidator $<
	date > $@

%.frag.test: %.frag
	glslangValidator $<
	date > $@


clean:
	-$(RM)  `find . -name "*.o"` ` find . -name "*.d"` $(COMPONENTS) rageOS.iso

.PHONY: all clean info debug debug-opt release default
