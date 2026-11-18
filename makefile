CXX = g++
HIPCC = hipcc

# Détection de la plateforme et de l'architecture GPU (AMD dédiés/intégrés ou NVIDIA)
HIP_PLATFORM ?= $(shell hipconfig --platform 2>/dev/null)

ifeq ($(HIP_PLATFORM), nvidia)
    GPU_ARCH ?= $(shell nvidia-smi --query-gpu=compute_cap --format=csv,noheader 2>/dev/null | head -n 1 | tr -d '.' | sed 's/^/sm_/')
    HIP_ARCH_FLAG = -arch=$(GPU_ARCH)
else
    # Filtre uniquement les GPU AMD dédiés/intégrés valides (évite gfx000)
    GPU_ARCH ?= $(shell offload-arch 2>/dev/null | grep -E '^gfx[1-9]' | head -n 1)
    ifeq ($(GPU_ARCH),)
        GPU_ARCH := $(shell rocminfo 2>/dev/null | grep -oE 'gfx[1-9][0-9a-z]+' | head -n 1)
    endif
    HIP_ARCH_FLAG = --offload-arch=$(GPU_ARCH)
endif

# Bascule automatique : GPU si hipcc + architecture détectée, sinon CPU (ex: Intel ou sans GPU)
ifeq ($(MODE),)
    ifneq ($(shell which $(HIPCC) 2>/dev/null),)
        ifneq ($(GPU_ARCH),)
            MODE = gpu
        else
            MODE = cpu
        endif
    else
        MODE = cpu
    endif
endif

BASE_CXXFLAGS = -std=c++23 -Wall -Wextra -O3 -Isrc -MMD -MP

ifeq ($(MODE), gpu)
    OUT_DIR = out/gpu
    CXXFLAGS = $(BASE_CXXFLAGS) -DUSE_GPU
    LINKER = $(HIPCC) $(HIP_ARCH_FLAG)
else
    OUT_DIR = out/cpu
    CXXFLAGS = $(BASE_CXXFLAGS)
    LINKER = $(CXX)
endif

SRC_DIR = src
TEST_DIR = test

SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")
HIP_FILES = $(shell find $(SRC_DIR) -name "*.hip")

OBJS = $(addprefix $(OUT_DIR)/, $(notdir $(SRC_FILES:.cpp=.o))) $(OUT_DIR)/test.o
ifeq ($(MODE), gpu)
    OBJS += $(addprefix $(OUT_DIR)/, $(notdir $(HIP_FILES:.hip=.o)))
endif

VPATH = $(shell find $(SRC_DIR) -type d)

TARGET = $(OUT_DIR)/diaphane

all: build

cpu:
	$(MAKE) build MODE=cpu

gpu:
	$(MAKE) build MODE=gpu

build: $(OUT_DIR) $(TARGET)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(TARGET): $(OBJS)
	$(LINKER) -O3 -o $@ $^

$(OUT_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT_DIR)/%.o: %.hip
	$(HIPCC) -O3 -Isrc $(HIP_ARCH_FLAG) -c $< -o $@

$(OUT_DIR)/test.o: $(TEST_DIR)/test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf out output.ppm

rebuild: clean all

.PHONY: all cpu gpu build clean rebuild

-include $(OBJS:.o=.d)