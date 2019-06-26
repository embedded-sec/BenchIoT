# This file include libraries/files specific to the targeted board current application. 
# It is included in the main Makefile. We also add IoT2 here

# extra libraries (LCD here and its dependencies)
EXT_LIB_ROOT = $(PROJDIR)lib/

# add IoT2 lib
ifeq ($(IOT2), 1)
EXT_LIBS_DIRS = $(EXT_LIB_ROOT)IoT2
endif

# EVAL_F469NI specific files
ifeq ($(BOARD), EVAL_F469NI)

EXT_LIBS_DIRS += $(EXT_LIB_ROOT)board-specific/STM/EVAL_F469NI/STM32469I_EVAL
EXT_LIBS_DIRS += $(EXT_LIB_ROOT)board-specific/STM/EVAL_F469NI/Components/mfxstm32l152
EXT_LIBS_DIRS += $(EXT_LIB_ROOT)/board-specific/STM/EVAL_F469NI/Components/cs43l22
EXT_LIBS_DIRS += $(EXT_LIB_ROOT)/board-specific/STM/EVAL_F469NI/Components/ft6x06
EXT_LIBS_DIRS += $(EXT_LIB_ROOT)/board-specific/STM/EVAL_F469NI/Components/otm8009a
EXT_LIBS_DIRS += $(EXT_LIB_ROOT)/board-specific/STM/EVAL_F469NI/Components/Common
EXT_LIBS_DIRS += $(EXT_LIB_ROOT)/board-specific/STM/EVAL_F469NI/Components/n25q128a
EXT_LIBS_DIRS += $(EXT_LIB_ROOT)/board-specific/STM/Utilities/Fonts
endif


#################### extra libs ###############################
EXT_LIBS_PATHS = $(patsubst %, -I%, $(EXT_LIBS_DIRS))

ifeq ($(BOARD), EVAL_F469NI)
EXT_LIBS_SRCS = $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/stm32469i_eval.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/stm32469i_eval_io.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/stm32469i_eval_sd.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/stm32469i_eval_lcd.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/stm32469i_eval_camera.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/stm32469i_eval_sdram.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/mfxstm32l152.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/otm8009a.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/ft6x06.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/cs43l22.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/font24.c))
endif


ifeq ($(IOT2), 1)
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/IoT2Lib.c))
endif


# *.o files for extra libraries only, will remain manual for the moment to avoid adding 
# unnecessary files.
C_EXT_LIBS_OBJS = $(patsubst %.c, $(BUILDDIR)%.o, $(filter %.c, $(notdir $(EXT_LIBS_SRCS))))
CXX_EXT_LIBS_OBJS += $(patsubst %.cpp, $(BUILDDIR)%.o, $(filter %.cpp, $(notdir $(EXT_LIBS_SRCS))))