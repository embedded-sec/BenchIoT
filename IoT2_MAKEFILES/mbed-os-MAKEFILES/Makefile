
################################################################################
#
# This is a makefile to build mbed-os independently of the application. It takes
# the macros defined by the application and build mbed-os accordingly.
#
################################################################################

#==========================   mbed paths  	  ==========================#

# Include mbed-os global variables auto-configuration file
include $(PROJDIR)IoT2_MAKEFILES/mbed-os-MAKEFILES/MBED_OS_GLOBAL_VARIABLES.mk


#========================== TOOLCHAIN specific ==========================#

# The setup of the toolchain is obtained from the TOOLCHAIN.mk file
#include $(PROJDIR)IoT2_MAKEFILES/TOOLCHAIN.mk


#==========================   mbed RULES  	  ==========================#


all : build

build: mbedlibstart $(C_OBJS) $(CXX_OBJS) $(ASM_OBJS) $(MBED_OBJS)$(APPNAME).ld mbedlibend

$(MBED_OBJS)%.o: %.c
	@echo "[+] Building mbed-os <*.c>: $(@F)"
	@$(CC) $(C_FLAGS) $(CC_DIRECTIVES) $(MBED_PATHS) -I$(BENCHDIR) $(EXT_LIBS_PATHS) $< -o $@

$(MBED_OBJS)%.o: %.cpp
	@echo "[+] Building mbed-os <*.cpp> : $(@F)"
	@$(CXX) $(CXX_FLAGS) $(CXX_DIRECTIVES) $(MBED_PATHS) -I$(BENCHDIR) $(EXT_LIBS_PATHS) $< -o $@
 
$(MBED_OBJS)%.o: %.S
	@echo "[+] Building mbed-os <*.S> : : $(@F)"
	@$(AS) $(ASM_FLAGS) $(MBED_PATHS) -I$(BENCHDIR) $(EXT_LIBS_PATHS) $< -o $@

mbedlibstart:
	@echo "[+] Creating $(MBED_OBJS) directory"
	@$(shell mkdir -p $(MBED_OBJS) 2>/dev/null)
	@echo "===------------------- 	BUILDING mbed 	-------------------==="

mbedlibend:
	@echo "===-------------------      DONE         -------------------==="

$(MBED_OBJS)$(APPNAME).ld: $(LINKER_SCRIPT)
	@$(CCPREP) $(PREPROC_FLAGS) $< -o $@


.PHONY: all


#========================== Dependency Rules ==========================#
include $(wildcard $(DEPDIR)/*)