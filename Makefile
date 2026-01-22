# Project Name
TARGET = PulsarVersio

# Sources
CPP_SOURCES = PulsarVersio.cpp PulsarEngine.cpp

# Library Locations - override with environment variables if needed
LIBDAISY_DIR ?= $(HOME)/src/libDaisy
DAISYSP_DIR ?= $(HOME)/src/DaisySP

# Optimization
OPT = -Os

# Core location, and generic Makefile
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
