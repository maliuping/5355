###################################################################
# Global feature options that need to align with Android ones
###################################################################
CFG_AUDIO_SUPPORT = no
CFG_MTK_AUDIO_TUNNELING_SUPPORT = no
CFG_MTK_AURISYS_PHONE_CALL_SUPPORT = no
CFG_MTK_VOW_SUPPORT = no
CFG_MTK_SPEAKER_PROTECTION_SUPPORT = no

###################################################################
# Malloc/free function support
###################################################################
CFG_MTK_MALLOC_SUPPORT=yes

###################################################################
# SCP internal feature options
###################################################################
CFG_TESTSUITE_SUPPORT = no
CFG_MODULE_INIT_SUPPORT = no
CFG_XGPT_SUPPORT = no
CFG_UART_SUPPORT = yes
CFG_MTK_SCPUART_SUPPORT = yes
CFG_SYSTMR_SUPPORT = yes
CFG_VIDEO_CORE_SUPPORT = yes
CFG_TVD_SUPPORT = yes
CFG_VIRTUAL_DISPLAY_SUPPORT = no
CFG_VIRTUAL_CAMERA_SUPPORT = no
CFG_VIRTUAL_M2M_SUPPORT = no
CFG_FASTRVC_SUPPORT = yes
CFG_FASTLOGO_SUPPORT = yes
CFG_FASTLOGO_ONLY = no
CFG_FASTRVC_GUIDELINE_SUPPORT = yes

# CFG_MTK_APUART_SUPPORT
# Do not use this with eng load or log may mix together and hard to recognzie
# Do not use this on lower power, it keeps infra always on
CFG_MTK_APUART_SUPPORT = no

##############################################################################
# When the option CFG_MTK_DYNAMIC_AP_UART_SWITCH is set to "yes", the code
# of UART will be built into the SCP image. This leads to a larger image.
# Set this option to "yes" only when you really know what you are doing.
# Otherwise, set it to "no".
##############################################################################
CFG_MTK_DYNAMIC_AP_UART_SWITCH = no
ifeq ($(CFG_MTK_DYNAMIC_AP_UART_SWITCH), yes)
CFG_UART_SUPPORT = yes
CFG_MTK_SCPUART_SUPPORT = no
CFG_MTK_APUART_SUPPORT = yes
endif

CFG_DSP_IPC_SUPPORT = yes
CFG_DSP_IPC_SUPPORT_TEST = no
CFG_DSP_IPC_LOG = no
CFG_DSP_IPC_WDT = no

CFG_SEM_SUPPORT = yes
CFG_IPC_SUPPORT = no
CFG_LOGGER_SUPPORT = no
CFG_LOGGER_TIMESTAMP_SUPPORT = no
CFG_LOGGER_BOOTLOG_SUPPORT = no
CFG_WDT_SUPPORT = no
CFG_DMA_SUPPORT = no
CFG_PMIC_WRAP_SUPPORT = no
CFG_ETM_SUPPORT = no
CFG_I2C_SUPPORT = yes
CFG_SPI_SUPPORT = no
CFG_CTP_SUPPORT = no
CFG_EINT_SUPPORT = no
CFG_GPIO_SUPPORT = yes
CFG_LVDS_SUPPORT = yes
CFG_RECOVERY_SUPPORT = no
CFG_HEAP_GUARD_SUPPORT = no
CFG_SENSORHUB_TEST_SUPPORT = no
CFG_VCORE_DVFS_SUPPORT = no
CFG_SENSORHUB_SUPPORT = no
CFG_MPU_DEBUG_SUPPORT = no
CFG_RAMDUMP_SUPPORT = no
CFG_DWT_SUPPORT = no
CFG_DRAMC_MONITOR_SUPPORT = no
CFG_CHRE_SUPPORT = no
CFG_CONTEXTHUB_FW_SUPPORT = no
CFG_CORE_BENCHMARK_SUPPORT = no
CFG_SLT_SUPPORT = no
CFG_DVT_SUPPORT = no
CFG_CCCI_DVT_SUPPORT = no
CFG_TICKLESS_SUPPORT = yes
CFG_FREERTOS_TRACE_SUPPORT = no
CFG_IRQ_MONITOR_SUPPORT = yes
CFG_IPI_STAMP_SUPPORT = no
CFG_OVERLAY_INIT_SUPPORT = no
CFG_OVERLAY_DEBUG_SUPPORT = no
CFG_OVERLAY2_SUPPORT = yes
CFG_PD_SUPPORT = yes
CFG_GCE_SUPPORT = yes
CFG_DISPLAY_SUPPORT = yes
CFG_MDP_SUPPORT = yes

CFG_FEATURE01_SUPPORT = no
CFG_FEATURE02_SUPPORT = no
CFG_FEATURE03_SUPPORT = no
CFG_CACHE_SUPPORT = no
CFG_CACHE_2WAY_SUPPORT = no
CFG_CM4_MODIFICATION = no
CFG_I2C_CH0_DMA_SUPPORT = no
CFG_I2C_CH1_DMA_SUPPORT = no
CFG_HEAP4_DRAM_SUPPORT = yes

CFG_CNN_TO_SCP_BUF_SIZE = 0x0
CFG_SCP_TO_CNN_BUF_SIZE = 0x0
###################################################################
# HW change
###################################################################
CFG_ONE_IPI_IRQ_DESIGN = no

###################################################################
# Optional ProjectConfig.mk used by project
###################################################################
-include $(PROJECT_DIR)/ProjectConfig.mk

###################################################################
# Mandatory platform-specific resources
###################################################################
INCLUDES += \
  $(PLATFORM_DIR)/inc \
  $(SOURCE_DIR)/kernel/service/common/include \
  $(SOURCE_DIR)/kernel/CMSIS/Device/MTK/$(PLATFORM)/Include \
  $(SOURCE_DIR)/middleware/SensorHub \
  $(DRIVERS_PLATFORM_DIR)/feature_manager/inc

C_FILES += \
  $(PLATFORM_DIR)/src/main.c \
  $(PLATFORM_DIR)/src/platform.c \
  $(PLATFORM_DIR)/src/interrupt.c \
  $(PLATFORM_DIR)/src/$(PLATFORM)_it.c \
  $(SOURCE_DIR)/kernel/service/common/src/mt_printf.c \

# Add startup files to build
C_FILES += $(PLATFORM_DIR)/CMSIS/system.c
S_FILES += $(PLATFORM_DIR)/CMSIS/startup.S

ifeq ($(CFG_DRAMC_MONITOR_SUPPORT), yes)
# Add dramc (gating auto save) files to build, and please DO NOT remove.
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dramc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dramc/dramc.c
endif

###################################################################
# Heap size config
###################################################################
ifeq ($(CFG_CHRE_256KB_SUPPORT),yes)
CFLAGS += -DconfigTOTAL_HEAP_SIZE='( ( size_t ) ( 35 * 1024 ) )'
else ifeq ($(CFG_MTK_AURISYS_PHONE_CALL_SUPPORT),yes)
CFLAGS += -DconfigTOTAL_HEAP_SIZE='( ( size_t ) ( 185 * 1024 ) )'
else ifeq ($(CFG_MTK_AUDIO_TUNNELING_SUPPORT),yes)
CFLAGS += -DconfigTOTAL_HEAP_SIZE='( ( size_t ) ( 165 * 1024 ) )'
else ifeq ($(CFG_MTK_SPEAKER_PROTECTION_SUPPORT),yes)
CFLAGS += -DconfigTOTAL_HEAP_SIZE='( ( size_t ) ( 130 * 1024 ) )'
else ifeq ($(CFG_CHRE_SUPPORT),yes)
CFLAGS += -DconfigTOTAL_HEAP_SIZE='( ( size_t ) ( 80 * 1024 ) )'
else
# default heap size
CFLAGS += -DconfigTOTAL_HEAP_SIZE='( ( size_t ) ( 30 * 1024 ) )'
endif

###################################################################
# Resources determined by configuration options
###################################################################
INCLUDES += -I$(SOURCE_DIR)/drivers/common/cache/v01/inc
ifeq ($(CFG_CACHE_SUPPORT), yes)
C_FILES  += $(SOURCE_DIR)/drivers/common/cache/v01/src/cache_internal.c
endif

ifeq ($(CFG_MTK_MALLOC_SUPPORT),yes)
C_FILES  += $(SOURCE_DIR)/kernel/service/common/src/mtk_malloc.c
LDFLAGS += -Wl,-wrap,malloc -Wl,-wrap,free
endif

ifeq ($(CFG_SLT_SUPPORT),yes)
INCLUDES += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_slt/inc
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_slt/src/slt.c
endif

ifeq ($(CFG_FREERTOS_TRACE_SUPPORT),yes)
C_FILES  += middleware/ondiemet/met_freertos.c
C_FILES  += middleware/ondiemet/met_main.c
C_FILES  += middleware/ondiemet/met_util.c
C_FILES  += middleware/ondiemet/met_tag.c
C_FILES  += middleware/ondiemet/met_log.c
INCLUDES += middleware/ondiemet/inc
CFLAGS  += -DMET_TAG
endif

ifeq ($(CFG_HEAP4_DRAM_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/middleware/heap_4/
C_FILES  += $(SOURCE_DIR)/middleware/heap_4/heap_4.c
endif

ifeq ($(CFG_TESTSUITE_SUPPORT),yes)
INCLUDES += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/inc
INCLUDES += $(SOURCE_DIR)/middleware/lib/console/include
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_sample/src/EINT_testsuite.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_sample/src/plat_testsuite.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_sample/src/sample.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_platform/src/platform.c
ifeq ($(CFG_VCORE_DVFS_SUPPORT),yes)
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_dvfs/src/dvfs_test.c
endif
ifeq ($(CFG_CCCI_DVT_SUPPORT),yes)
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/ccif_dvt.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/ccif_dvt_test_lib_ap.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/ts_ccif_dvt.c
endif
ifeq ($(CFG_I2C_SUPPORT),yes)
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_sample/src/i2c_testsuite.c
endif
C_FILES  += middleware/lib/console/console.c
endif

ifeq ($(CFG_CORE_BENCHMARK_SUPPORT),yes)
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_list_join.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_main.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_matrix.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_state.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_util.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_portme.c
CFLAGS   += -DPERFORMANCE_RUN=1 -DITERATIONS=500 -DBUILD_WITH_FREERTOS
endif

ifeq ($(CFG_MODULE_INIT_SUPPORT),yes)
C_FILES  += $(SOURCE_DIR)/kernel/service/common/src/module_init.c
endif

ifeq ($(CFG_OVERLAY_INIT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/overlay/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/overlay/src/mtk_overlay_init.c
endif

ifeq ($(CFG_OVERLAY2_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/ovl2
C_FILES  += $(DRIVERS_PLATFORM_DIR)/ovl2/ovl2.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/ovl2/ovl2_hw.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/ovl2/wdma2_hw.c
endif

ifeq ($(CFG_XGPT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/xgpt/inc/
C_FILES  += $(DRIVERS_PLATFORM_DIR)/xgpt/src/xgpt.c
C_FILES  += $(SOURCE_DIR)/kernel/service/common/src/utils.c
endif

ifeq ($(CFG_PMIC_WRAP_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/drivers/common/pmic_wrap/v2/inc \
  $(DRIVERS_PLATFORM_DIR)/pmic_wrap/inc
C_FILES  += \
   $(SOURCE_DIR)/drivers/common/pmic_wrap/v2/pmic_wrap.c
endif

ifeq ($(CFG_UART_SUPPORT),yes)
C_FILES  += $(SOURCE_DIR)/drivers/common/uart/v01/uart.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/uart/uart_platform.c
endif

ifeq ($(CFG_I2C_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/i2c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/i2c/mtk_i2c.c
endif

ifeq ($(CFG_GCE_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/gce
C_FILES  += $(DRIVERS_PLATFORM_DIR)/gce/mtk-cmdq-control.c \
  $(DRIVERS_PLATFORM_DIR)/gce/mtk-cmdq-helper.c
endif

ifeq ($(CFG_MDP_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/gce \
  $(DRIVERS_PLATFORM_DIR)/mdp
C_FILES  += $(DRIVERS_PLATFORM_DIR)/mdp/mdp.c
endif

ifeq ($(CFG_SEM_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/sem/
C_FILES  += $(DRIVERS_PLATFORM_DIR)/sem/scp_sem.c
endif

C_FILES  += $(DRIVERS_PLATFORM_DIR)/smi/smi.c

ifeq ($(CFG_IPC_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/ipi/
C_FILES  += $(DRIVERS_PLATFORM_DIR)/ipi/scp_ipi.c
endif

ifeq ($(CFG_DSP_IPC_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/drivers/common/ipi \
            $(SOURCE_DIR)/drivers/common/ipi/uid_queue \
            $(SOURCE_DIR)/drivers/common/ipi/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/ipi/scp_ipi.c \
            $(SOURCE_DIR)/drivers/common/ipi/uid_queue/scp_uid_queue.c \
            $(SOURCE_DIR)/drivers/common/ipi/uid_queue/uid_queue_task.c
endif

ifeq ($(CFG_DSP_IPC_SUPPORT_TEST),yes)
INCLUDES += $(SOURCE_DIR)/drivers/common/ipi/sample
C_FILES  += $(SOURCE_DIR)/drivers/common/ipi/sample/test.c
endif

ifeq ($(CFG_DSP_IPC_LOG),yes)
INCLUDES += $(SOURCE_DIR)/drivers/common/ipi/log
C_FILES  += $(SOURCE_DIR)/drivers/common/ipi/log/ipi_log.c
endif

ifeq ($(CFG_DSP_IPC_WDT),yes)
INCLUDES += $(SOURCE_DIR)/drivers/common/ipi/wdt
C_FILES  += $(SOURCE_DIR)/drivers/common/ipi/wdt/ipi_wdt.c
endif

ifeq ($(CFG_LOGGER_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/logger/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/logger/v01/src/scp_logger.c
endif

ifeq ($(CFG_WDT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/wdt/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/wdt/src/wdt.c
endif

ifeq ($(CFG_SYSTMR_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/systmr/
C_FILES  += $(DRIVERS_PLATFORM_DIR)/systmr/mt_systmr.c
endif

ifeq ($(CFG_VIDEO_CORE_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/video-core/
C_FILES  += $(DRIVERS_PLATFORM_DIR)/video-core/video_core.c
ifeq ($(CFG_VIRTUAL_DISPLAY_SUPPORT),yes)
C_FILES  += $(DRIVERS_PLATFORM_DIR)/video-core/sample_virtual_display.c
endif
ifeq ($(CFG_VIRTUAL_CAMERA_SUPPORT),yes)
C_FILES  += $(DRIVERS_PLATFORM_DIR)/video-core/sample_virtual_camera.c
endif
ifeq ($(CFG_VIRTUAL_M2M_SUPPORT),yes)
C_FILES  += $(DRIVERS_PLATFORM_DIR)/video-core/sample_virtual_m2m.c
endif
endif

ifeq ($(CFG_TVD_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/tvd
C_FILES  += $(DRIVERS_PLATFORM_DIR)/tvd/tvd_if.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/tvd/tvd_drv.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/tvd/tvd_setting.c
endif
ifeq ($(CFG_FASTRVC_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/middleware/fastrvc/inc
C_FILES  += $(SOURCE_DIR)/middleware/fastrvc/src/fastrvc_main.c
C_FILES  += $(SOURCE_DIR)/middleware/fastrvc/src/module.c
C_FILES  += $(SOURCE_DIR)/middleware/fastrvc/src/module_base.c
C_FILES  += $(SOURCE_DIR)/middleware/fastrvc/src/module_disp.c
C_FILES  += $(SOURCE_DIR)/middleware/fastrvc/src/module_ovl.c
endif

ifeq ($(CFG_DMA_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dma
C_FILES  += $(SOURCE_DIR)/drivers/common/dma/v03/src/dma.c
C_FILES  += $(SOURCE_DIR)/drivers/common/dma/v03/src/dma_api.c
endif

ifeq ($(CFG_ETM_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/etm
C_FILES  += $(DRIVERS_PLATFORM_DIR)/etm/etm.c
endif

#ifeq ($(CFG_CTP_SUPPORT),yes)
#endif

ifeq ($(CFG_EINT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/eint/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/eint/v02/src/eint.c
endif

ifeq ($(CFG_GPIO_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/inc
C_FILES  += $(SOURCE_DIR)/drivers/CM4_A/mt2712/gpio/gpio.c
endif

ifeq ($(CFG_LVDS_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/inc
C_FILES  += $(SOURCE_DIR)/drivers/CM4_A/mt2712/display/mtk_dpi.c
C_FILES  += $(SOURCE_DIR)/drivers/CM4_A/mt2712/display/mtk_lvds.c
C_FILES  += $(SOURCE_DIR)/drivers/CM4_A/mt2712/panel/panel-simple.c
endif

ifeq ($(CFG_HEAP_GUARD_SUPPORT),yes)
C_FILES  += $(RTOS_SRC_DIR)/portable/MemMang/mtk_HeapGuard.c
LDFLAGS += -Wl, -wrap=pvPortMalloc -Wl, -wrap=vPortFree
endif

ifeq ($(CFG_VCORE_DVFS_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/kernel/service/common/include/
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dvfs/inc
INCLUDES += $(SOURCE_DIR)/drivers/common/dvfs/v01/inc
INCLUDES += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/inc/
C_FILES  += $(SOURCE_DIR)/drivers/common/dvfs/v01/src/dvfs_common.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dvfs/src/dvfs.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dvfs/src/sleep.c
endif

ifeq ($(CFG_SENSORHUB_SUPPORT), yes)
INCLUDES += $(HAL_PLATFORM_DIR)/driver/ipi/inc
INCLUDES += $(SOURCE_DIR)/middleware/lib/console/include
INCLUDES += $(SOURCE_DIR)/middleware/SensorHub
INCLUDES += $(SOURCE_DIR)/kernel/FreeRTOS/Source/include
INCLUDES += $(PLATFORM_DIR)/inc
INCLUDES += $(SOURCE_DIR)/kernel/FreeRTOS/Source/portable/GCC/ARM_CM4F
INCLUDES += $(SOURCE_DIR)/kernel/service/common/include
C_FILES  += $(SOURCE_DIR)/middleware/SensorHub/sensor_manager.c
C_FILES  += $(SOURCE_DIR)/middleware/SensorHub/sensor_manager_fw.c
endif

ifeq ($(CFG_SENSORHUB_TEST_SUPPORT), yes)
C_FILES  += $(SOURCE_DIR)/middleware/SensorHub/sensorframeworktest.c
C_FILES  += $(SOURCE_DIR)/middleware/SensorHub/FakeAccelSensorDriver.c
endif

ifeq ($(CFG_FLP_SUPPORT), yes)
CFG_GEOFENCE_SUPPORT = yes
endif
ifeq ($(CFG_GEOFENCE_SUPPORT), yes)
#CFG_CCCI_SUPPORT = yes
endif
ifeq ($(CFG_CCCI_SUPPORT), yes)
INCLUDES += -I$(DRIVERS_PLATFORM_DIR)/ccci
INCLUDES += -I$(SOURCE_DIR)/drivers/common/ccci
C_FILES  += $(SOURCE_DIR)/drivers/common/ccci/ccci.c
C_FILES  += $(SOURCE_DIR)/drivers/common/ccci/ccism_ringbuf.c
C_FILES  += $(SOURCE_DIR)/drivers/common/ccci/sensor_modem.c
C_FILES  += $(SOURCE_DIR)/drivers/common/ccci/ccci_hw_ccif.c
endif

ifeq ($(CFG_MPU_DEBUG_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/mpu/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/mpu/v01/src/mpu_mtk.c
endif


ifeq ($(CFG_MTK_AUDIO_TUNNELING_SUPPORT),yes)
CFG_AUDIO_SUPPORT = yes
endif

ifeq ($(CFG_MTK_VOW_SUPPORT),yes)
CFG_AUDIO_SUPPORT = yes
endif

# add for spk_protection
ifeq ($(CFG_MTK_SPEAKER_PROTECTION_SUPPORT),yes)
CFG_AUDIO_SUPPORT = yes
endif

ifeq ($(CFG_AUDIO_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/middleware/lib/aurisys/interface
INCLUDES += $(SOURCE_DIR)/middleware/lib/audio_utility
INCLUDES += $(SOURCE_DIR)/drivers/common/audio/framework
INCLUDES += $(SOURCE_DIR)/drivers/common/audio/hardware
INCLUDES += $(SOURCE_DIR)/drivers/common/audio/tasks
INCLUDES += $(DRIVERS_PLATFORM_DIR)/audio/hardware
C_FILES  += $(SOURCE_DIR)/middleware/lib/audio_utility/audio_ringbuf.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_messenger_ipi.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_task_factory.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/audio/hardware/audio_hw.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/audio/hardware/audio_irq.c
endif

ifeq ($(CFG_CHRE_SUPPORT), yes)
INCLUDES += -I$(DRIVERS_PLATFORM_DIR)/i2c/inc/contexthub
INCLUDES += -I$(SOURCE_DIR)/../../../hardware/contexthub/firmware/inc
include $(FEATURE_CONFIG_DIR)/chre.mk
endif

ifeq ($(CFG_DWT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dwt/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dwt/src/dwt.c
endif

# add for speaker_protection
ifeq ($(CFG_MTK_SPEAKER_PROTECTION_SUPPORT),yes)
INCLUDES += \
  $(SOURCE_DIR)/drivers/common/audio/tasks/spkprotect \
  $(SOURCE_DIR)/middleware/lib/aurisys/dummylib \
  $(SOURCE_DIR)/middleware/lib/audio_utility
C_FILES += \
  $(SOURCE_DIR)/drivers/common/audio/tasks/spkprotect/audio_task_speaker_protection.c
  LIBFLAGS += -Wl,-L$(SOURCE_DIR)/middleware/lib/aurisys/dummylib,-ldummylib
endif

include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_MTK_AUDIO_TUNNELING_SUPPORT
FEATURE_NAME   := FEATURE_MTK_AUDIO_TUNNELING
FEATURE_INCLUDES := \
  $(SOURCE_DIR)/middleware/lib/mp3offload/inc \
  $(SOURCE_DIR)/middleware/lib/blisrc/inc \
  $(SOURCE_DIR)/drivers/common/audio/tasks/mp3 \
  $(DRIVERS_PLATFORM_DIR)/audio/tasks/mp3
FEATURE_C_FILES := \
  $(SOURCE_DIR)/drivers/common/audio/tasks/mp3/audio_do_mp3.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/mp3/audio_task_offload_mp3.c \
  $(DRIVERS_PLATFORM_DIR)/audio/tasks/mp3/audio_dma_mp3.c
FEATURE_LIBFLAGS := \
  -L$(SOURCE_DIR)/middleware/lib/mp3offload -lmp3dec \
  -L$(SOURCE_DIR)/middleware/lib/blisrc -lblisrc
include $(REGISTER_FEATURE)
include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_MTK_VOW_SUPPORT
FEATURE_NAME   := FEATURE_MTK_VOW
FEATURE_INCLUDES := \
  $(SOURCE_DIR)/middleware/lib/vow/inc \
  $(DRIVERS_PLATFORM_DIR)/audio/hardware \
  $(SOURCE_DIR)/drivers/common/audio/tasks/vow/v03 \
  $(SOURCE_DIR)/drivers/common/audio/hardware/vow/v02 \
  $(DRIVERS_PLATFORM_DIR)/audio/tasks/vow
FEATURE_C_FILES := \
  $(SOURCE_DIR)/drivers/common/audio/tasks/vow/v03/audio_do_vow.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/vow/v03/audio_task_vow.c \
  $(DRIVERS_PLATFORM_DIR)/audio/hardware/vow_hw.c \
  $(DRIVERS_PLATFORM_DIR)/audio/tasks/vow/audio_dma_vow.c \
  $(SOURCE_DIR)/middleware/lib/vow/swVAD.c
FEATURE_LIBFLAGS := \
  -L$(SOURCE_DIR)/middleware/lib/vow -lvow \
  -L$(SOURCE_DIR)/middleware/lib/vow -lblisrc_vow
include $(REGISTER_FEATURE)


include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_FEATURE01_SUPPORT
FEATURE_NAME   := FEATURE01
FEATURE_INCLUDES := \
  $(SOURCE_DIR)/middleware/feature01 \
  $(SOURCE_DIR)/middleware/feature01_task
FEATURE_C_FILES := \
  $(SOURCE_DIR)/middleware/feature01/feature01.c
FEATURE_SCP_C_FILES := \
  $(SOURCE_DIR)/middleware/feature01_task/feature01_sample.c
include $(REGISTER_FEATURE)

include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_FEATURE02_SUPPORT
FEATURE_NAME   := FEATURE02
FEATURE_INCLUDES := \
  $(SOURCE_DIR)/middleware/feature02
FEATURE_C_FILES := \
  $(SOURCE_DIR)/middleware/feature02/feature02.c
include $(REGISTER_FEATURE)

include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_FEATURE03_SUPPORT
FEATURE_NAME   := FEATURE03
FEATURE_INCLUDES := \
  $(SOURCE_DIR)/middleware/feature03
FEATURE_C_FILES := \
  $(SOURCE_DIR)/middleware/feature03/feature03.c
include $(REGISTER_FEATURE)

ifeq ($(CFG_DISPLAY_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/display/
C_FILES  += $(DRIVERS_PLATFORM_DIR)/display/mtk_display_drv.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/display/mtk_display_ctx.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/display/mtk_display_ddp.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/display/mtk_display_ddp_comp.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/display/mtk_display_ovl.c
endif

###################################################################
# Optional Dynamic Object definition file that includes all
# supported Dynamic Object sets
###################################################################
-include $(PLATFORM_DIR)/dos.mk

###################################################################
# Optional CompilerOption.mk used by project
###################################################################
-include $(PROJECT_DIR)/CompilerOption.mk
ifeq ($(CFG_TICKLESS_SUPPORT),yes)
CFLAGS += -DconfigUSE_TICKLESS_IDLE=1
else
CFLAGS += -DconfigUSE_TICKLESS_IDLE=0
endif

###################################################################
# Post processing
###################################################################
CFG_DO_ENABLED :=
ifneq (,$(strip $($(PROCESSOR).ALL_DOS)))
CFG_DO_ENABLED = yes
INCLUDES += $(DO_COMMON_INCLUDES)
C_FILES += $(DO_SCP_C_FILES)
S_FILES += $(DO_SCP_S_FILES)
endif
