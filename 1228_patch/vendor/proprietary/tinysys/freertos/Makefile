###########################################################
## Generic definitions
###########################################################
# Remove $@ if error occurs
.DELETE_ON_ERROR:

# Turn off suffix build rules built into make
.SUFFIXES:

.PHONY: FORCE
FORCE:

SHELL          := /bin/bash
.DEFAULT_GOAL  := all

TINYSYS_SCP    := tinysys-scp
TINYSYS_LOADER := tinysys-loader
TINYSYS_DO     := tinysys-dos

###########################################################
## Parameter control
###########################################################
PROJECT := $(strip $(PROJECT))
ifeq ($(PROJECT),)
  $(error $(TINYSYS_SCP): project name is required)
endif

ifeq ($(strip $(O)),)
O := $(TINYSYS_SCP)_out
endif

INSTALLED_DIR ?= $(O)

# Verbosity control
# Add 'V=1' with make or 'showcommands' when building Android
V ?= 1
ifeq ($(V),1)
hide :=
else
hide := @
endif

###########################################################
## Common directory locations and generic variables
###########################################################
SOURCE_DIR         := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
BUILT_DIR          := $(O)/freertos/source

LOCAL_DIR          := $(PWD)
DRIVERS_DIR        := $(SOURCE_DIR)/drivers
DRIVERS_COMMON_DIR := $(DRIVERS_DIR)/common
BUILD_DIR          := $(SOURCE_DIR)/build
TOOLS_DIR          := $(SOURCE_DIR)/tools
RTOS_SRC_DIR       := $(SOURCE_DIR)/kernel/FreeRTOS/Source
BASE_DIR           := $(SOURCE_DIR)/project
DO_SERVICE_DIR     := $(SOURCE_DIR)/middleware/DoService
DO_PICS_DIR        := $(SOURCE_DIR)/middleware/fastrvc/pics
TINYSYS_SCP_BIN    := $(INSTALLED_DIR)/scp.img
TINYSYS_SCP_DATA_BIN   := $(INSTALLED_DIR)/scp-data.img
TINYSYS_SECURE_DIR := $(SOURCE_DIR)/../secure
SCP_LOGO_IMAGE     := $(DO_PICS_DIR)/mediatek_logo_1920x1080.BGR
SCP_WARNING_IMAGE  := $(DO_PICS_DIR)/mediatek_warning_1920x1080.BGR
SCP_GUIDELING      := $(DO_PICS_DIR)/mediatek_guideline_1920x1080.BGRA
TINYSYS_FIT        := $(DO_PICS_DIR)/scpdata.its
SCPSYS_FIT         := $(BUILD_DIR)/scpsys.its
SCPSYS_KEY_DIR     := $(LOCAL_DIR)/device/mediatek/mt2712/security/keys
KEY_DIR            := $(BUILT_DIR)/key-scp

MKIMAGE            := $(TOOLS_DIR)/mkimage
MKIMAGE_UBOOT      := $(TOOLS_DIR)/mkimage_uboot
OBJSIZE            := $(TOOLS_DIR)/objsize
MCHECK             := $(TOOLS_DIR)/memoryReport.py
SCP_DATA_COMPRESS  := $(LOCAL_DIR)/prebuilts/misc/linux-x86/lz4/lz4

# Common functions and utilities
include $(BUILD_DIR)/toolchain.mk
include $(BUILD_DIR)/definitions.mk

# Initialize the environment for each processor
include $(BUILD_DIR)/main.mk

ifeq (1,$(V))
  $(info $(TINYSYS_SCP): PROCESSORS=$(PROCESSORS))
  $(info $(TINYSYS_SCP): PROJECT=$(PROJECT))
  $(info $(TINYSYS_SCP): PLATFORM=$(PLATFORM))
  $(info $(TINYSYS_SCP): O=$(O))
  $(info $(TINYSYS_SCP): SOURCE_DIR=$(SOURCE_DIR))
  $(info $(TINYSYS_SCP): ALL_SCP_BINS=$(ALL_SCP_BINS))
endif

SETTING            := $(BASE_DIR)/CM4_A/$(PLATFORM)/platform/Setting.ini

###########################################################
## Build targets
###########################################################
all: $(TINYSYS_SCP_BIN) $(TINYSYS_SCP_DATA_BIN);

SORTED_SCP_BINS := $(call sort_tinysys_binaries,$(ALL_SCP_BINS))

$(TINYSYS_SCP_BIN): $(SORTED_SCP_BINS)
	@mkdir -p $(dir $@)
	@echo '$(TINYSYS_SCP): BIN   $@'
	@if [ -s '$($(PROCESSOR)_DRAM.TINYSYS_BIN_BUILT)' ]; then \
		$(hide) cat $^ $($(PROCESSOR)_DRAM.TINYSYS_BIN_BUILT) > $@; \
	else \
		$(hide) cat $^ > $@; \
	fi;
	cp $(SCPSYS_FIT) $(BUILT_DIR)/scpsys.its
	mkdir -p $(KEY_DIR)
	cp -f $(SCPSYS_KEY_DIR)/verified_key.crt $(KEY_DIR)/dev.crt
	cp -f $(SCPSYS_KEY_DIR)/verified_key.pem $(KEY_DIR)/dev.key
	cp -f $($(PROCESSOR).BIN_NH) $(BUILT_DIR)/scpsys.bin
	$(BUILD_DIR)/lz4_compress.sh $(BUILT_DIR)/scpsys.bin $(SCP_DATA_COMPRESS)
	rm -rf $(TINYSYS_SCP_BIN)
	$(MKIMAGE_UBOOT) -k $(KEY_DIR) -f $(BUILT_DIR)/scpsys.its $(TINYSYS_SCP_BIN)
	rm -rf $(BUILT_DIR)/scpsys.its
	rm -rf $(BUILT_DIR)/scpsys.bin
	rm -rf $(BUILT_DIR)/scpsys.bin.lz4

$(TINYSYS_SCP_DATA_BIN):
	@echo "mkimage $(TINYSYS_SCP_DATA_BIN) start"
	@echo $(MKIMAGE)
	@echo "$(SCP_LOGO_IMAGE)"
	cp $(SCP_LOGO_IMAGE) $(BUILT_DIR)/scp-logo
	$(BUILD_DIR)/lz4_compress.sh $(BUILT_DIR)/scp-logo $(SCP_DATA_COMPRESS)
	cp $(SCP_WARNING_IMAGE) $(BUILT_DIR)/scp-warning
	$(BUILD_DIR)/lz4_compress.sh $(BUILT_DIR)/scp-warning $(SCP_DATA_COMPRESS)
	cp $(SCP_GUIDELING) $(BUILT_DIR)/scp-guideline
	$(BUILD_DIR)/lz4_compress.sh $(BUILT_DIR)/scp-guideline $(SCP_DATA_COMPRESS)
	cp $(TINYSYS_FIT) $(BUILT_DIR)/scpdata.its
	$(MKIMAGE_UBOOT) -f $(BUILT_DIR)/scpdata.its $(TINYSYS_SCP_DATA_BIN)
	rm -rf $(BUILT_DIR)/scp-logo
	rm -rf $(BUILT_DIR)/scp-warning
	rm -rf $(BUILT_DIR)/scp-guideline
	rm -rf $(BUILT_DIR)/scpdata.its

clean:
	rm -rf $(TINYSYS_SCP_BIN) $(O)
	rm -rf $(TINYSYS_SCP_DATA_BIN) $(O)
