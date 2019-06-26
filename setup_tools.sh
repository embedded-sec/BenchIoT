#!/bin/bash

################################################################################
#
# This file is used to setup symbolic links for external libraries so that
# they are added in a unified method wether we are using mbed-os or mbed-sdk.
#
################################################################################


################################## VARIABLS ##################################

CURR_DIR=`dirname \`readlink -f $0\``
TOOLS='tools'
TOOLS_DIR=${CURR_DIR}/${TOOLS}
GENERAL_LIB_DIR=${CURR_DIR}/lib

SYM_LINK='ln -sTf'

##################################  SETUP   ##################################

 

# Create tools directory
#mkdir -p ${TOOLS_DIR}
cd ${TOOLS_DIR}

# Pull a version of the rop gadgets compiler
if git clone  git@github.com:JonathanSalwan/ROPgadget.git
then
	echo "[+] Setup completed: ROPgadget compiler"
else
	echo "[-] ERROR: Failed to clone ROPgadget compiler"
fi


