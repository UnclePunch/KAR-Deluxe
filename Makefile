ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment.")
endif

ifeq ($(OS),Windows_NT)
    PYTHON := python
else
    PYTHON := python3
endif

# --- Compiler and Flags ---
CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc
LD = $(DEVKITPPC)/bin/powerpc-eabi-ld

# --- Directories ---
BUILD_DIR 		= build
SCRIPT_DIR 		?= scripts
HOSHI_DIR		= externals/hoshi
LIB_ROOT_DIR 	= $(HOSHI_DIR)/Lib
INC_DIR 		?= $(HOSHI_DIR)/include
PACKTOOL_DIR 	?= $(HOSHI_DIR)/packtool
HOSHI_BIN_DIR	= $(HOSHI_DIR)/out/release
ORIG_DOL		= $(HOSHI_DIR)/dol/kar.dol
ROOT_DIR 		= root
ISO_DIR		 	?= iso
OUT_DIR 		= out
MODS_OUT_DIR 	= $(ROOT_DIR)/files/mods
ISO_OUT_DIR 	= 
INSTALL_DIR 	?=

# --- File Paths ---
ISO_PATH		= kar.iso
HOSHI_BIN		= $(HOSHI_BIN_DIR)/hoshi.bin

# --- Script Paths ---
ISOPATCH_SCRIPT		= $(SCRIPT_DIR)/iso.py
DOLEXTRACT_SCRIPT	= $(SCRIPT_DIR)/dol.py

# User-defined CFLAGS.
CFLAGS = -O1 -mcpu=750 -meabi -msdata=none -mhard-float -ffreestanding \
           -fno-unwind-tables -fno-exceptions -fno-asynchronous-unwind-tables \
           -fno-merge-constants -ffunction-sections -fdata-sections \
           -MMD # needed for automatic dependency generation

LDFLAGS  ?= -r -T$(PACKTOOL_DIR)/link.ld

# Define MODS_ROOT_DIR
MODS_ROOT_DIR = mods

# --- Derived Variables ---
# INCLUDES: Transforms include paths into compiler -I flags
INCLUDES = -I$(INC_DIR) -I$(LIB_ROOT_DIR)

# --- Source File Discovery ---

# 1. Libraries: Find all C source files recursively under the LIB_ROOT_DIR.
LIB_SOURCES := $(shell find $(LIB_ROOT_DIR) -name "*.c")

# 2. Mods: Find all mods in the mod folder
MOD_NAMES ?= $(notdir $(wildcard $(MODS_ROOT_DIR)/*))
#MOD_NAMES = city_settings credits

# 3. Mods Source: For each mod, find its specific source files within its 'src' subdirectory.
MOD_C_SOURCES := $(foreach mod,$(MOD_NAMES),\
                       $(shell find $(MODS_ROOT_DIR)/$(mod)/src -name "*.c"))
MOD_ASM_SOURCES := $(foreach mod,$(MOD_NAMES),\
                       $(shell find $(MODS_ROOT_DIR)/$(mod)/src -name "*.s"))

# --- Object and Dependency File Mapping ---

# Map individual library source files to their corresponding object files in BUILD_DIR.
LIB_OBJECTS := $(patsubst $(LIB_ROOT_DIR)/%.c,$(BUILD_DIR)/$(LIB_ROOT_DIR)/%.o,$(LIB_SOURCES))

# Map individual mod source files to their corresponding object files in BUILD_DIR.
MOD_C_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(MOD_C_SOURCES))
MOD_ASM_OBJECTS := $(patsubst %.s,$(BUILD_DIR)/%.o,$(MOD_ASM_SOURCES))

MOD_OBJECTS		:= $(MOD_C_OBJECTS) $(MOD_ASM_OBJECTS)

# Combine ALL individual object files (from both libraries and mods) that need to be compiled.
ALL_INDIVIDUAL_OBJECTS_TO_COMPILE = $(LIB_OBJECTS) $(MOD_C_OBJECTS) $(MOD_ASM_OBJECTS)

# Map all these compiled objects to their corresponding dependency files (.d files).
DEPS := $(ALL_INDIVIDUAL_OBJECTS_TO_COMPILE:.o=.d)

# Get a list of all unique build directories that need to be created for ALL objects.
OBJ_DIRS := $(sort $(dir $(ALL_INDIVIDUAL_OBJECTS_TO_COMPILE)))

# --- Mod Specific Linked Objects & Binaries ---

# MOD_LINKED_OBJECTS: The output of the linking step for each mod (e.g. build/credits.o)
MOD_LINKED_FILES := $(addsuffix .modlink, $(addprefix $(BUILD_DIR)/, $(MOD_NAMES)))

# MOD_BIN_FILES: The final .bin files for each mod (e.g. out/credits.bin)
MOD_BIN_FILES := $(addsuffix .bin, $(addprefix $(MODS_OUT_DIR)/, $(MOD_NAMES)))

# Define a variable for all asset directories
MOD_ASSET_DIRS := $(foreach mod,$(MOD_NAMES),\
                               $(if $(wildcard $(MODS_ROOT_DIR)/$(mod)/assets), \
                                    $(MODS_ROOT_DIR)/$(mod)/assets))

# --- Debug Outputs ---
# $(warning DEBUG: MOD_ASSET_DIRS = $(MOD_ASSET_DIRS))
#$(warning DEBUG: LIB_SOURCES = $(LIB_SOURCES))
#$(warning DEBUG: LIB_OBJECTS = $(LIB_OBJECTS))
#$(warning DEBUG: MOD_NAMES = $(MOD_NAMES))
#$(warning DEBUG: MOD_ALL_SOURCES = $(MOD_ALL_SOURCES))
#$(warning DEBUG: MOD_OBJECTS = $(MOD_OBJECTS))
#$(warning DEBUG: ALL_INDIVIDUAL_OBJECTS_TO_COMPILE = $(ALL_INDIVIDUAL_OBJECTS_TO_COMPILE))
#$(warning DEBUG: MOD_LINKED_FILES = $(MOD_LINKED_FILES))
#$(warning DEBUG: DEPS = $(DEPS))

# --- Main Targets ---

.PHONY: all clean install assets riivolution patch

# The 'all' target builds all final .bin files.
all: 		$(MOD_BIN_FILES) hoshi assets
package: 	all riivolution patch

# --- Directory Creation Rules ---
# Rule to create the top-level build directory
$(BUILD_DIR):
	@mkdir -p $@

# Rule to create the mods output directory
$(OUT_DIR):
	@mkdir -p $@

# Rule to create the mods output directory
$(MODS_OUT_DIR):
	@mkdir -p $@

# Rule to create all necessary subdirectories within the build folder for objects
$(OBJ_DIRS):
	@mkdir -p $@

# Rule to extract the original dol from the iso
$(ORIG_DOL):
	$(PYTHON) $(DOLEXTRACT_SCRIPT) $(ISO_PATH) $(ORIG_DOL)

# --- hoshi target ---
hoshi: $(ORIG_DOL)
	$(MAKE) -C $(HOSHI_DIR) MOD_NAME=KirbyAirRideDeluxe

# --- Generic Compilation Rule for C Source Files ---
# This single pattern rule handles compiling ANY .c file into its corresponding .o file in BUILD_DIR.
# It uses an order-only prerequisite to ensure the output directory exists before compilation.
$(BUILD_DIR)/%.o: %.c | $(OBJ_DIRS)
#	@echo "Compiling $<..."
	@mkdir -p $(dir $@) 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | $(OBJ_DIRS)
#	@echo "Compiling $<..."
	@mkdir -p $(dir $@) 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# --- Linking Rules for Individual Mod Files ---
# Define a macro to get the mod-specific object files for linking.
# $(1) is the mod name (e.g., 'credits').
define GET_MOD_LINK_OBJECTS
$(filter $(BUILD_DIR)/$(MODS_ROOT_DIR)/$(1)/src/%.o, $(MOD_OBJECTS))
endef

# Define a template for the linking rule (including recipe).
# This template will be used for each mod.
define LINK_MOD_RULE_TEMPLATE
$(BUILD_DIR)/$(1).modlink: $(LIB_OBJECTS) $(call GET_MOD_LINK_OBJECTS,$(1))
#	@echo ""
#	@echo "--- Linking '$(1)' Mod Object Files ---"
#	@echo "DEBUG (Linking Rule): All prerequisites ($$^): $$^" # See what $$^ contains
#	@echo "DEBUG (Linking Rule): Filtered prerequisites for linker: $(filter %.o,$$^)" # See what is passed to linker
#	@echo ""
	$(LD) $(LDFLAGS) $$^ -o $$@
endef

# Generate a specific linking rule for each mod using the template.
$(foreach mod,$(MOD_NAMES),\
  $(eval $(call LINK_MOD_RULE_TEMPLATE,$(mod))))

# --- Packing Rule for Bin Files ---
# Define a template for the packing rule (including recipe).
# This template will be used for each mod.
define PACK_MOD_RULE_TEMPLATE
$(MODS_OUT_DIR)/$(1).bin: $(BUILD_DIR)/$(1).modlink | $(MODS_OUT_DIR)
	@echo ""
	@echo "--- Creating '$(1)' bin file ---"
	@echo ""
	$(PYTHON) $(PACKTOOL_DIR)/main.py $$< -m gbFunction -o $$@

endef

# Generate a specific linking rule for each mod using the template.
$(foreach mod,$(MOD_NAMES),\
  $(eval $(call PACK_MOD_RULE_TEMPLATE,$(mod))))

# --- Include generated dependency files (.d files) ---
-include $(DEPS)

# Rule to copy all assets to the root directory
assets: hoshi
	@for dir in $(MOD_ASSET_DIRS); do \
		if [ -d "$$dir" ]; then \
			cp -a "$$dir"/* "$(ROOT_DIR)/files/"; \
		fi; \
	done
	cp -a -r "$(ISO_DIR)"/* "$(strip $(ROOT_DIR))/"
	cp -a -r "$(HOSHI_DIR)/dol/out"/main.dol "$(strip $(ROOT_DIR)/sys)/"
	cp -a -r "$(HOSHI_DIR)/out/release"/* "$(strip $(ROOT_DIR)/files)/"

# --- Install Target ---
# Copies the files from $(ROOT_DIR) to $(INSTALL_DIR)
install: $(MOD_BIN_FILES) hoshi assets 
	@echo ""
	$(MAKE) -C $(HOSHI_DIR) install INSTALL_DIR="$(INSTALL_DIR)"
	@echo "--- Installing files to "$(INSTALL_DIR)" ---"
	cp -a -r "$(ROOT_DIR)"/* "$(strip $(INSTALL_DIR))/"
	@echo ""
	@echo "Installation complete."

patch: $(OUT_DIR) $(MOD_BIN_FILES) hoshi assets
	@echo ""
	@echo "--- Creating ISO Patch... ---"
	python $(ISOPATCH_SCRIPT) $(ISO_PATH) $(ROOT_DIR) $(OUT_DIR)/patch.xdelta

riivolution: $(OUT_DIR) $(MOD_BIN_FILES) hoshi assets
	@echo ""
	@echo "--- Creating Riivolution Mod... ---"
	cp -a -r "$(HOSHI_DIR)/dol/out/Riivolution" "$(OUT_DIR)"
	cp -a -r "$(ROOT_DIR)"/files/* "$(OUT_DIR)/Riivolution/KirbyAirRideDeluxe"

# --- Clean Target ---
clean:
	@echo "Cleaning build and output directories..."
	$(MAKE) -C $(HOSHI_DIR) clean
	rm -rf $(ORIG_DOL) $(ROOT_DIR) $(OUT_DIR) $(BUILD_DIR)