###########################################################
## Check for valid float argument
##
## NOTE that you have to run make clean after changing
## these since hardfloat and softfloat are not binary
## compatible
###########################################################
ifneq ($(FLOAT_TYPE), hard)
  ifneq ($(FLOAT_TYPE), soft)
    override FLOAT_TYPE = hard
    #override FLOAT_TYPE = soft
  endif
endif

ifeq ($(FLOAT_TYPE), hard)
  FPUFLAGS = -fsingle-precision-constant -Wdouble-promotion
  FPUFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
  #CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
else
  FPUFLAGS = -msoft-float
endif

ALLFLAGS := -g -Os -Wall -Werror -mlittle-endian -mthumb -Wno-error=format
CFLAGS := \
  -flto -ffunction-sections -fdata-sections -fno-builtin \
  -D_REENT_SMALL $(FPUFLAGS) -gdwarf-2 -std=c99

# Add to support program counter backtrace dump
CFLAGS   += -funwind-tables

# Define build type. Default to release
BUILD_TYPE ?= release

# DO ID max length
DO_ID_MAX_LENGTH := 32

ifeq (1,$(V))
  $(info $(TINYSYS_SCP): BUILD_TYPE=$(BUILD_TYPE))
endif

ifeq (debug,$(strip $(BUILD_TYPE)))
  CFLAGS   += -DTINYSYS_DEBUG_BUILD
endif

###########################################################
## LD flags
###########################################################
LDFLAGS := \
  $(FPUFLAGS) \
  --specs=nano.specs -lc -lnosys -lm \
  -nostartfiles --specs=rdimon.specs -lrdimon -Wl,--gc-sections

# Add to support printf wrapper function
LDFLAGS  += -Wl,-wrap,printf

###########################################################
## Processor-specific common instructions
###########################################################
ifneq ($(filter CM4_%,$(PROCESSOR)),)
  PROCESSOR_FLAGS := -mcpu=cortex-m4
  ALLFLAGS        += $(PROCESSOR_FLAGS)
  CFLAGS          += $(ALLFLAGS)
  LDFLAGS         += $(ALLFLAGS)
endif

###########################################################
## Common source codes
###########################################################
RTOS_FILES := \
  $(RTOS_SRC_DIR)/tasks.c \
  $(RTOS_SRC_DIR)/list.c \
  $(RTOS_SRC_DIR)/queue.c \
  $(RTOS_SRC_DIR)/timers.c \
  $(RTOS_SRC_DIR)/event_groups.c \
  $(RTOS_SRC_DIR)/portable/MemMang/heap_4.c

###########################################################
## Include path
###########################################################
INCLUDES := \
  $(RTOS_SRC_DIR)/include \
  $(SOURCE_DIR)/$(APP_PATH) \
  $(SOURCE_DIR)/kernel/CMSIS/Include

###########################################################
## Processor-specific common instructions
###########################################################
ifneq ($(filter CM4_%,$(PROCESSOR)),)
  RTOS_FILES += $(RTOS_SRC_DIR)/portable/GCC/ARM_CM4F/port.c
  INCLUDES   += $(RTOS_SRC_DIR)/portable/GCC/ARM_CM4F
endif

C_FILES := $(RTOS_FILES)

###########################################################
## Feature directives when used in DO
###########################################################
#FEATURE_COMMON_CFLAGS   := $(filter-out -funwind-tables,$(CFLAGS))
FEATURE_COMMON_CFLAGS   := $(CFLAGS)

###########################################################
## Common directives for DO
###########################################################
DO_PSEUDO_ENTRY    := __do_pseudo_startup_entry
DO_LINKER_SCRIPT   := $(DO_SERVICE_DIR)/dynamic_object.rel.ld
DO_COMMON_INCLUDES := $(DO_SERVICE_DIR)
# Files whose built objects will be linked in SCP image if DO is enabled
DO_SCP_C_FILES     := $(patsubst %,$(DO_SERVICE_DIR)/%, \
  do_service.c \
  dynamic_object.c \
  exidx_dynamic_object.c \
  verify.c)
DO_SCP_S_FILES     :=
# Source files whose built objects will be linked into each DO instance
DO_GENERIC_C_FILES  := $(DO_SERVICE_DIR)/pseudo_startup_entry.c
DO_GENERIC_S_FILES  :=
DO_STRIP_OPTIONS    := --strip-debug --strip-unneeded
DO_OBJCOPY_OPTIONS  := -O binary
#DO_COMMON_LDFLAGS   := $(filter-out -Wl$(COMMA)--gc-sections,$(LDFLAGS)) -Wl,-r -Wl,-T$(DO_LINKER_SCRIPT)
DO_COMMON_LDFLAGS   := -Wl,-r -Wl,--entry=$(DO_PSEUDO_ENTRY) -Wl,-T$(DO_LINKER_SCRIPT) $(LDFLAGS)
DO_ELFTOOLS_DIR     := $(TOOLS_DIR)/pyelftools
MKDOHEADER          := $(TOOLS_DIR)/mkdoheader
DO_PYTHON_ENV       := PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=$(DO_ELFTOOLS_DIR)
