# --- Compiler and Flags ---
CC = powerpc-eabi-gcc
LD = powerpc-eabi-ld

# --- Directories ---
BUILD_DIR 		= build
HOSHI_DIR		= externals/hoshi
LIB_ROOT_DIR 	= $(HOSHI_DIR)/Lib
INC_DIR 		?= $(HOSHI_DIR)/include
TOOL_DIR 		?= $(HOSHI_DIR)/packtool
OUT_DIR 		= out
INSTALL_DIR 	?= C:/Users/Vin/Documents/ROMs/KAR-Plus/files/mods		#can override this on the command line: make install INSTALL_DIR=/path/to/your/mods

# User-defined CFLAGS.
CFLAGS = -O1 -mcpu=750 -meabi -msdata=none -mhard-float -ffreestanding \
           -fno-unwind-tables -fno-exceptions -fno-asynchronous-unwind-tables \
           -fno-merge-constants -ffunction-sections -fdata-sections \
           -MMD # needed for automatic dependency generation

LDFLAGS  ?= -r -T$(TOOL_DIR)/link.ld

# Define MODS_ROOT_DIR
MODS_ROOT_DIR = mods

# --- Derived Variables ---
# INCLUDES: Transforms include paths into compiler -I flags
INCLUDES = -I$(INC_DIR) -I$(LIB_ROOT_DIR)

# --- Source File Discovery ---

# 1. Libraries: Find all C source files recursively under the LIB_ROOT_DIR.
LIB_SOURCES := $(shell find $(LIB_ROOT_DIR) -name "*.c")

# 2. Mods: Manually list your mod folder names.
MOD_NAMES ?= $(shell find $(MODS_ROOT_DIR) -maxdepth 1 -mindepth 1 -type d -printf "%f\n")
#MOD_NAMES = city_settings credits

# 3. Mods: For each mod, find its specific source files within its 'src' subdirectory.
MOD_C_SOURCES := $(foreach mod,$(MOD_NAMES),\
                       $(shell find $(MODS_ROOT_DIR)/$(mod)/src -name "*.c"))
MOD_ASM_SOURCES := $(foreach mod,$(MOD_NAMES),\
                       $(shell find $(MODS_ROOT_DIR)/$(mod)/src -name "*.s"))

# --- Object and Dependency File Mapping ---

# Map individual library source files to their corresponding object files in BUILD_DIR.
LIB_OBJECTS := $(patsubst $(LIB_ROOT_DIR)/%.c,$(BUILD_DIR)/$(LIB_ROOT_DIR)/%.o,$(LIB_SOURCES))

# Map individual mod source files to their corresponding object files in BUILD_DIR.
MOD_C_OBJECTS 	:= $(patsubst $(MODS_ROOT_DIR)/%.c,$(BUILD_DIR)/$(MODS_ROOT_DIR)/%.o,$(MOD_C_SOURCES))
MOD_ASM_OBJECTS := $(patsubst $(MODS_ROOT_DIR)/%.s,$(BUILD_DIR)/$(MODS_ROOT_DIR)/%.o,$(MOD_ASM_SOURCES))
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
MOD_BIN_FILES := $(addsuffix .bin, $(addprefix $(OUT_DIR)/, $(MOD_NAMES)))

# --- Debug Outputs ---
#$(warning DEBUG: LIB_SOURCES = $(LIB_SOURCES))
#$(warning DEBUG: LIB_OBJECTS = $(LIB_OBJECTS))
#$(warning DEBUG: MOD_NAMES = $(MOD_NAMES))
#$(warning DEBUG: MOD_ALL_SOURCES = $(MOD_ALL_SOURCES))
#$(warning DEBUG: MOD_OBJECTS = $(MOD_OBJECTS))
#$(warning DEBUG: ALL_INDIVIDUAL_OBJECTS_TO_COMPILE = $(ALL_INDIVIDUAL_OBJECTS_TO_COMPILE))
#$(warning DEBUG: MOD_LINKED_FILES = $(MOD_LINKED_FILES))
#$(warning DEBUG: DEPS = $(DEPS))

# --- Main Targets ---

.PHONY: all clean install

# The 'all' target builds all final .bin files.
all: $(MOD_BIN_FILES)

# --- Directory Creation Rules ---
# Rule to create the top-level build directory
$(BUILD_DIR):
	@mkdir -p $@

# Rule to create the top-level output directory
$(OUT_DIR):
	@mkdir -p $@

# Rule to create all necessary subdirectories within the build folder for objects
$(OBJ_DIRS):
	@mkdir -p $@

# --- Generic Compilation Rule for C Source Files ---
# This single pattern rule handles compiling ANY .c file into its corresponding .o file in BUILD_DIR.
# It uses an order-only prerequisite to ensure the output directory exists before compilation.
$(BUILD_DIR)/%.o: %.c
	@echo ""
	@echo "Compiling C file: $<..."
	@echo ""
	@mkdir -p $(dir $@) 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@echo ""
	@echo "Compiling ASM file: $<..."
	@echo ""
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
	@echo ""
	@echo "--- Linking '$(1)' Mod Object Files ---"
	@echo "DEBUG (Linking Rule): All prerequisites ($$^): $$^" # See what $$^ contains
	@echo "DEBUG (Linking Rule): Filtered prerequisites for linker: $(filter %.o,$$^)" # See what is passed to linker
	@echo ""
	$(LD) $(LDFLAGS) $$^ -o $$@
endef

# Generate a specific linking rule for each mod using the template.
$(foreach mod,$(MOD_NAMES),\
  $(eval $(call LINK_MOD_RULE_TEMPLATE,$(mod))))

# --- Packing Rule for Bin Files ---
# Define a template for the linking rule (including recipe).
# This template will be used for each mod.
define PACK_MOD_RULE_TEMPLATE
$(OUT_DIR)/$(1).bin: $(BUILD_DIR)/$(1).modlink $(OUT_DIR)
	@echo ""
	@echo "--- Creating '$(1)' bin file ---"
	@echo ""
	python $(TOOL_DIR)/main.py $$< -m gbFunction -o $$@

endef

# Generate a specific linking rule for each mod using the template.
$(foreach mod,$(MOD_NAMES),\
  $(eval $(call PACK_MOD_RULE_TEMPLATE,$(mod))))

# --- Include generated dependency files (.d files) ---
-include $(DEPS)

# --- Install Target ---
# Copies the final .bin files from $(OUT_DIR) to $(INSTALL_DIR)
install: all
	@echo ""
	@echo "--- Installing mod binaries to "$(INSTALL_DIR)" ---"
	cp -r "$(OUT_DIR)"/* "$(strip $(INSTALL_DIR))/"
	@echo ""
	@echo "Installation complete."

# --- Clean Target ---
clean:
	@echo "Cleaning build and output directories..."
	$(RM) -r $(BUILD_DIR)
	$(RM) -r $(OUT_DIR)