# ==========================
# Compiler & flags
# ==========================
CXX := C:/MinGW/bin/g++.exe
CC  := C:/MinGW/bin/gcc.exe
CXXFLAGS := -std=c++17 -Wall -Wextra -g -D_CRT_SECURE_NO_WARNINGS -DIMGUI_DISABLE_WIN32_FUNCTIONS
LDFLAGS  := -Llib
LIBS     := -lglad -lglfw3dll -lopengl32

# ==========================
# Directories
# ==========================
SRC     := src
INCLUDE := include
LIB     := lib
OUTPUT  := output
MAIN    := main.exe

# ==========================
# ImGui sources
# ==========================
IMGUI_SRC := \
    $(INCLUDE)/imgui/imgui.cpp \
    $(INCLUDE)/imgui/imgui_draw.cpp \
    $(INCLUDE)/imgui/imgui_tables.cpp \
    $(INCLUDE)/imgui/imgui_widgets.cpp

IMGUI_BACKEND := \
    $(INCLUDE)/imgui/backends/imgui_impl_glfw.cpp \
    $(INCLUDE)/imgui/backends/imgui_impl_opengl3.cpp

# ==========================
# Project sources
# ==========================
PROJECT_SRC := $(wildcard $(SRC)/*.cpp)

# ==========================
# All sources & objects
# ==========================
SOURCES := $(PROJECT_SRC) $(IMGUI_SRC) $(IMGUI_BACKEND) $(INCLUDE)/stb_image.cpp
OBJECTS := $(SOURCES:.cpp=.o)
DEPS    := $(OBJECTS:.o=.d)
INCLUDES := -I$(INCLUDE) -I$(INCLUDE)/imgui -I$(INCLUDE)/imgui/backends

# ==========================
# Output
# ==========================
OUTPUT_MAIN := $(OUTPUT)/$(MAIN)

# ==========================
# Make rules
# ==========================
all: $(OUTPUT) $(OUTPUT_MAIN)

$(OUTPUT):
	mkdir $(OUTPUT)

$(OUTPUT_MAIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

# Compile .cpp -> .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -MMD $< -o $@

# Include dependencies
-include $(DEPS)

# Clean
.PHONY: clean
clean:
	del /Q /F $(OUTPUT_MAIN)
	del /Q /F $(OBJECTS)
	del /Q /F $(DEPS)

# Run
.PHONY: run
run: all
	$(OUTPUT_MAIN)
