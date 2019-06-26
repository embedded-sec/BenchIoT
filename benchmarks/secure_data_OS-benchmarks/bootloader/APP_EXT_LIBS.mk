# This file include libraries/files specific to the current application. It is included in the
# main Makefile.

# extra libraries (LCD here and its dependencies) [debug]
EXT_LIB_ROOT = $(PROJDIR)lib/

# add IoT2 lib
ifeq ($(IOT2), 1)
EXT_LIBS_DIRS = $(EXT_LIB_ROOT)IoT2
endif

ifeq ($(BOARD), EVAL_F469NI)
EXT_LIBS_DIRS += $(EXT_LIB_ROOT)/board-specific/STM/EVAL_F469NI/STM32469I_EVAL
EXT_LIBS_DIRS += $(EXT_LIB_ROOT)/board-specific/STM/EVAL_F469NI/Components/mfxstm32l152
endif



#################### extra libs ###############################
EXT_LIBS_PATHS = $(patsubst %, -I%, $(EXT_LIBS_DIRS))


#EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/IoT2Lib.c))


ifeq ($(BOARD), EVAL_F469NI)
EXT_LIBS_SRCS = $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/stm32469i_eval.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/stm32469i_eval_io.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/stm32469i_eval_camera.c))
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/mfxstm32l152.c))
endif


ifeq ($(IOT2), 1)
EXT_LIBS_SRCS += $(foreach dir, $(EXT_LIBS_DIRS), $(wildcard $(dir)/IoT2Lib.c))
endif

# *.o files for extra libraries only, will remain manual for the moment to avoid adding
# unnecessary files.
C_EXT_LIBS_OBJS = $(patsubst %.c, $(BUILDDIR)%.o, $(filter %.c, $(notdir $(EXT_LIBS_SRCS))))
CXX_EXT_LIBS_OBJS += $(patsubst %.cpp, $(BUILDDIR)%.o, $(filter %.cpp, $(notdir $(EXT_LIBS_SRCS))))
