
################################################################################
#
# This is used to automatically include the display werapper class of a specific
# board. Note that other board specific files are added in the APP_EXT_LIBS.mk
# file. If such files are added here make sure they are removed from the 
# APP_EXT_LIBS.mk file.
#
################################################################################


# Include the file of global variables for mbed libraries
include $(PROJDIR)IoT2_MAKEFILES/mbed-libs-MAKEFILES/MBED_LIBS_GLOBAL_VARIABLES.mk

EVAL_F469NI_BSP_DIRS += $(EXT_LIB_ROOT)board-specific/STM/EVAL_F469NI/STM32469I_EVAL
EVAL_F469NI_BSP_DIRS += $(EXT_LIB_ROOT)board-specific/STM/EVAL_F469NI/Components/mfxstm32l152
EVAL_F469NI_BSP_DIRS += $(EXT_LIB_ROOT)board-specific/STM/EVAL_F469NI/Components/cs43l22
EVAL_F469NI_BSP_DIRS += $(EXT_LIB_ROOT)board-specific/STM/EVAL_F469NI/Components/ft6x06
EVAL_F469NI_BSP_DIRS += $(EXT_LIB_ROOT)board-specific/STM/EVAL_F469NI/Components/otm8009a
EVAL_F469NI_BSP_DIRS += $(EXT_LIB_ROOT)board-specific/STM/EVAL_F469NI/Components/Common
EVAL_F469NI_BSP_DIRS += $(EXT_LIB_ROOT)board-specific/STM/EVAL_F469NI/Components/n25q128a
EVAL_F469NI_BSP_DIRS += $(EXT_LIB_ROOT)board-specific/STM/Utilities/Fonts

# Add the source files needed. For now, all src files are added by default.
# To optimize, each file can be added manually by changing the * ---> <filename>
# note that you all need to duplicate the rule for each file added in that case.
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/stm32469i_eval.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/stm32469i_eval_io.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/stm32469i_eval_sd.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/stm32469i_eval_lcd.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/stm32469i_eval_camera.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/stm32469i_eval_sdram.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/mfxstm32l152.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/otm8009a.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/ft6x06.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/cs43l22.c))
EVAL_F469NI_BSP_C_SRCS += $(foreach dir,$(EVAL_F469NI_BSP_DIRS), $(wildcard $(dir)/font24.c))

EVAL_F469NI_BSP_CXX_SRCS = $(foreach dir,$(EVAL_F469NI_BSP_DIRS),$(wildcard $(dir)/*.cpp))


# add sd lib to mbed lib dirs
MBED_LIBS_DIRS += $(EVAL_F469NI_BSP_DIRS)

# Add the library to the global mbed libs srcs
MBED_LIBS_C_SRCS += $(EVAL_F469NI_BSP_C_SRCS)
MBED_LIBS_CXX_SRCS += $(EVAL_F469NI_BSP_CXX_SRCS)


# The object files are created in the calling makefile (whether it is mbed-os 
# or mbed-sdk).