
################################################################################
#
# This is an autoconfig file for toolchains supported by IoT2. 
#
################################################################################


#========================== TOOLCHAIN specific ==========================#

ARM_EABI := arm-none-eabi-
CC = $(ARM_EABI)gcc
CXX = $(ARM_EABI)g++
AS = $(ARM_EABI)gcc -x assembler-with-cpp
LD = $(ARM_EABI)ld
AR = $(ARM_EABI)ar
CCPREP = $(ARM_EABI)cpp
OBJCOPY = $(ARM_EABI)objcopy