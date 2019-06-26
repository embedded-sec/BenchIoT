
################################################################################
#
# This is a makefile to build mbed-sdk independently of the application. It takes
# the macros defined by the application and build mbed-sdk accordingly.
#
################################################################################

#==========================   mbed paths  	  ==========================#

# Include mbed-sdk global variables auto-configuration file
include $(PROJDIR)IoT2_MAKEFILES/mbed-sdk-MAKEFILES/MBED_SDK_GLOBAL_VARIABLES.mk


#==========================   mbed RULES  	  ==========================#


all : build

build: mbedlibstart $(C_OBJS) $(CXX_OBJS) $(ASM_OBJS) $(MBED_OBJS)$(APPNAME).ld mbedlibend

$(MBED_OBJS)%.o: %.c
	@echo "[+] Building mbed-sdk <*.c>: $(@F)"
	@$(CC) $(C_FLAGS) $(CC_DIRECTIVES) $(MBED_PATHS) -I$(BENCHDIR) $(EXT_LIBS_PATHS) $< -o $@

$(MBED_OBJS)%.o: %.cpp
	@echo "[+] Building mbed-sdk <*.cpp> : $(@F)"
	@$(CXX) $(CXX_FLAGS) $(CXX_DIRECTIVES) $(MBED_PATHS) -I$(BENCHDIR) $(EXT_LIBS_PATHS) $< -o $@
 
$(MBED_OBJS)%.o: %.S
	@echo "[+] Building mbed-sdk <*.S> : : $(@F)"
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