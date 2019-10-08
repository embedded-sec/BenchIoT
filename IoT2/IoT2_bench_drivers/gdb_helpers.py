

'''
This is a helper file used to ease debugging with gdb
'''

import csv
from texttable import Texttable
import gdb
import time
import json
import subprocess

ISCR_LUT = ['Thread mode',
            'Reserved',
            'NMI',
            'HardFault',
            'MemManage',
            'BusFault',
            'UsageFault',
            'Reserved',
            'Reserved',
            'Reserved',
            'Reserved',
            'SVCall',
            'Reserved for Debug',
            'Reserved',
            'PendSV',
            'SysTick']

def get_print_num(in_str):
    p_str = gdb.execute('p /x %s'%(in_str),to_string=True)
    return int(p_str.split("=")[1],16)

def get_value_from_addr(addr):
    ret_str = gdb.execute('p /x *0x%08x'%addr,to_string=True)
    return int(ret_str.split()[-1].strip(),16)


def get_mem(addr):
    res = gdb.execute('x 0x%x'%addr, to_string = True)
    (addr,value) = res.split(":")
    value = int(value.split('x')[1].strip(),16)
    return value

def set_mem(addr,value):
    res = gdb.execute('set *(uint32_t*)0x%x = 0x%x'%(addr,value))

def get_stacked_pc(stackoffset=0):
    '''
        Gets the PC pushed on the stack from in an ISR
        Offset can be used adjust if additional things have been
        pushed to stack
    '''
    sp = get_print_num('$sp')
    return get_mem(sp+(4*6)+stackoffset)

def parse_hardfault(hardfault,sp_offset):
    print "Hard Fault 0x%x Reason: "%hardfault,
    if hardfault &(1<<30):
        print "Forced--Other fault elavated"
    if hardfault & (1<<1):
        print "Bus Fault"
    print "Stacked PC 0x%x" %(get_stacked_pc(sp_offset))

BFAR = 0xE000ED38
MMAR = 0xE000ED34

def parse_cfsr(cfsr,sp_offset):
    print "CFSR 0x%x"%cfsr
    print "MemManage Flags"
    if cfsr & (1<<7):
        print "\tMemManage Fault Address Valid: 0x%x" % get_mem(MMAR)
    if cfsr & (1<<5):
        print "\tMemManage fault occurred during floating-point lazy state preservation"
    if cfsr & (1<<4):
        print "\tStacking for an exception entry has caused one or more access violations"
    if cfsr & (1<<3):
        print "\tUnstacking for an exception return has caused one or more access violations"
    if cfsr & (1<<1):
        print "\tData Access, Stacked PC 0x%x, Faulting Addr 0x%x" %(get_stacked_pc(sp_offset), get_mem(MMAR))
    if cfsr & (1):
        print "\tInstruction Access Violation, Stacked PC 0x%x" %(get_stacked_pc(sp_offset))
    print "BusFault:"
    if cfsr & (1<<15):
        print "\t Bus Fault Addr Valid 0x%x" % get_mem(BFAR)
    if cfsr & (1<<13):
        print "\tbus fault occurred during"
    if cfsr & (1<<12):
        print "\tException Stacking fault"
    if cfsr & (1<<11):
        print "\tException UnStacking fault"
    if cfsr & (1<<10):
        print "\tImprecise data bus error, may not have location"
    if cfsr & (1<<9):
        print "\tPrecise data bus error, Faulting Addr: %0x"%get_mem(BFAR)
    if cfsr & (1<<8):
        print "\tInstruction bus error"

    print "Other Faults"
    if cfsr & (1<<(9+16)):
        print "\tDiv by zero, Stacked PC has Addr"
    if cfsr & (1<<(8+16)):
        print "\tUnaligned Fault Stacking fault"
    if cfsr & (1<<(3+16)):
        print "\tNo Coprocessor"
    if cfsr & (1<<(2+16)):
        print "\tInvalid PC load UsageFault, Stacked PC has Addr"
    if cfsr & (1<<(1+16)):
        print "\tInvalid state UsageFault, Stacked PC has Addr"
    if cfsr & (1<<(16)):
        print "\tUndefined instruction UsageFault, Stacked PC has Addr"

def print_exception_stack(offset=0):
    '''
        Prints registers pushed on the stack by exception entry
        http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0553a/Babefdjc.html
    '''
    sp = get_print_num('$sp')
    sp += offset
    print "Registers Stacked by Exception"
    print "R0: 0x%x" % get_mem(sp)
    print "R1: 0x%x" % get_mem(sp+4)
    print "R2: 0x%x" % get_mem(sp+8)
    print "R3: 0x%x" % get_mem(sp+12)
    print "R12: 0x%x" % get_mem(sp+16)
    print "LR: 0x%x" % get_mem(sp+20)
    print "PC: 0x%x" % get_mem(sp+24)
    print "xPSR: 0x%x" % get_mem(sp+28)
    #TODO Check CCR for floating point and print S0-S15 FPSCR

def print_hardfault_info(stack_offset =0):
    '''
        Prints Hardfault info, alias for print_hardfault_info
    '''
    print ("Configurable Fault Status Reg")
    hardfault_status = get_mem(0xE000ED2C)
    print_exception_stack(stack_offset)
    parse_hardfault(hardfault_status,stack_offset)

    cfsr = get_mem(0xE000ED28)
    parse_cfsr(cfsr,stack_offset)

def hf(stack_offset = 0):
    '''
        Prints Hardfault info, alias for print_hardfault_info
    '''
    print_hardfault_info(stack_offset)

def hfb(stack_offset = 0):
    '''
        Prints Hardfault info and sets break point on faulting pc
    '''
    hf(stack_offset)
    pc =  get_stacked_pc(stack_offset)
    gdb.execute('b *0x%x'%pc)

# See ARM Documentation for details
#http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0553a/Cihjddef.html
MPU_CTRL = 0xE000ED94
MPU_RNR = 0xE000ED98
MPU_RBAR = 0xE000ED9C
MPU_RASR = 0xE000EDA0
PER_LUT ={0:'(P-,U-)', 1:'(P-RW,U-)', 2:'(P-RW,U-R)', 3:'(P-RW,U-RW)',
          4:'(Undef)', 5:'(P-R,U-)', 6:'(P-R,U-R)', 7:'(P-R,U-R)'
          }
def decode_mpu(rbar,rasr):
    AP = (rasr >> 24)&0x7
    M = (rasr>>1)&0x1F
    print "\tRegion %i" %(rbar&0xf)
    start = rbar&(~0x1F)
    print "\tStart: 0x%x"%(start)
    size = 2**(M+1)
    print "\tEnd: 0x%x"%(start+size-1)
    print "\tSize: %i, 0x%x" % (size,size)
    print "\tPermissions 26:24: 0x%x ,%s" %(AP,PER_LUT[AP])
    print "\tExecute Never: ",((rasr&(0x1<<28))>>28)
    print "\tSCB 18:16, 0x%x:" %((rasr&(0x7<<16))>>16)
    print "\tTEX 21:19, 0x%x:" %((rasr&(0x7<<19))>>19)
    print "\tEnabled :",(rasr&0x1)
    print "RBAR: 0x%x" %rbar
    print "RASR: 0x%x" %rasr
    print "-"*40

def print_mpu_ctrl_reg():
    mpu_ctrl = get_mem(MPU_CTRL)
    print "MPU_CTRL: 0x%x"%(mpu_ctrl)
    print "\tEnabled: ", (mpu_ctrl & 0x1)
    print "\tEnabled During NMI/HF: ", ((mpu_ctrl>>1) & 0x1)
    print "\tPriv Has Default Map:", ((mpu_ctrl>>2) & 0x1)

def print_mpu_config():

    for i in range(8):
        print "Reading %i" %i
        set_mem(MPU_RNR,i)
        rbar = get_mem(MPU_RBAR)
        rasr = get_mem(MPU_RASR)
        decode_mpu(rbar,rasr)
    print_mpu_ctrl_reg()

def mpu():
    print_mpu_config()

'''
    The below functions are specific to the k64f MPU should provide the same information as the functions above. The
    functions will follow the same naming convention with '_k64f' appended to it.
    For detauls about the MPU check the refrence manual at:
    http://cache.freescale.com/files/microcontrollers/doc/ref_manual/K64P144M120SF5RM.pdf?fpsp=1&WT_TYPE=Reference%20Manuals&WT_VENDOR=FREESCALE&WT_FILE_FORMAT=pdf&WT_ASSET=Documentation&fileExt=.pdf

    There are more registers the used below for the MPU (the below is NOT comprehensive by any means and is NOT reliable)

    What is implemented below is the following:

    The board supports 12 regions with each region having 4 registers defining its (start address, end address, permissions, valid flag)

    To check these is gdb it is simply x/48x 0x4000d400 (0x4000d400 is the address of the first register for the first region)
    what you will see is the following:

                [START]     [END]       [Perm.]     [Valid or not]
    0x4000d400:	0x00000000	0xffffffff	0x000007d0	0x00000001      <---- This is the first regions
    0x4000d410:	0x00000000	0x00024b7f	0x0010400d	0x00000001      <---- This is the second regions
    0x4000d420:	0x20000400	0x2003001f	0x00186017	0x00000001      <---- This is the 3rd regions
    0x4000d430:	0x1fff6100	0x1fff651f	0x0000001e	0x00000001      <---- ...etc
    0x4000d440:	0x1fff6580	0x1fff7eff	0x0000001e	0x00000001
    0x4000d450:	0x00000000	0x0000001f	0x00000000	0x00000000
    0x4000d460:	0x00000000	0x0000001f	0x00000000	0x00000000
    0x4000d470:	0x00000000	0x0000001f	0x00000000	0x00000000
    0x4000d480:	0x00000000	0x0000001f	0x00000000	0x00000000
    0x4000d490:	0x00000000	0x0000001f	0x00000000	0x00000000
    0x4000d4a0:	0x00000000	0x0000001f	0x00000000	0x00000000
    0x4000d4b0:	0x00000000	0x0000001f	0x00000000	0x00000000


    Quick Notes:
        - To change the regions you might need to use the '_ALT' registers, refer to the manual for more details.
        - When 2 regions overlap, the shared region is defined by the OR of their respective permissions.

'''

MPU_CESR = 0x4000D000       #Control/Error Status Register
MPU_RGDi_WORDi_base = 0x4000D400
PER_LUT_K64F_USER ={0:'(U-)', 1:'(U-X)', 2:'(U-W)', 3:'(U-WX)',
          4:'(U-R)', 5:'(U-RX)', 6:'(U-RW)', 7:'(U-RWX)'
          }
PER_LUT_K64F_PRIV ={0:'(P-RWX)', 1:'(P-RX)', 2:'(P-RW)', 3:'(P-SAME AS USER)'}
# this the same as mpu(), but specific to the k64f board since it has a different MPU
def mpu_k64f():
    region_base = MPU_RGDi_WORDi_base
    config_table = Texttable()
    config_list = []
    config_list.append(['Region', 'Start', 'End', 'Permissions', 'Valid'])
    for i in range(12):
        st = get_mem(region_base)
        ed = get_mem(region_base+4)
        perm = get_mem(region_base+8)
        valid_flag = get_mem(region_base+12) & 1
        perm_u = perm & 0x7
        perm_p = (perm >> 3) & 0x3
        perm_str = PER_LUT_K64F_USER[perm_u] + PER_LUT_K64F_PRIV[perm_p]
        config_list.append([str(i), str(hex(st)), str(hex(ed)), perm_str, str(valid_flag)])
        region_base += 0x10

    config_table.add_rows(config_list)
    print(config_table.draw())
    # print("---------------------------------------------------------------")


def get_isr():
    '''
    Prints the interrupt that caused the interrupt.
    Useful when hit default handler and do not know why
    '''
    iscr = get_mem(0xE000ED04) #This is ISCR
    isr = iscr & 0x1FF
    if isr <len(ISCR_LUT):
        print "%i: %s"%(isr,ISCR_LUT[isr])
    else:
        print "%i: IRQ%i"%(isr,isr-16)


def connect():
    gdb.execute('target remote localhost:3333')
    gdb.execute('monitor reset halt')

def load():
    gdb.execute('load')

def cl():
    'Short connect and load'
    connect()
    load()
    gdb.execute('monitor arm semihosting enable')

def c():
    'short connect for load us py c()'
    connect()

def clm():
    'short connect break on main'
    cl()
    gdb.execute('b main')

def mrh():
    'short for monitor reset halt'
    gdb.execute('monitor reset halt')

def mr():
    'short for monitor reset'
    gdb.execute('monitor reset')

# returns the current pc value
def getpc():
    res = gdb.execute('x $pc', to_string=True)
    res = res.split()
    curr_pc = res[0]
    return curr_pc

# end the gdb debugging session, useful when running experiments automatically
def close_gdb():
    gdb.execute('disconnect')
    gdb.execute('quit')


#######################################################################################################
#                                    Performance Measurement                                          #
#######################################################################################################


'''
These functions are to count cycles executed using
DWT_CTRL and DWT_* registers. See ARM's documentation
for more details.
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0439c/BABJFFGJ.html
'''

DWT_CTRL = 0xE0001000
DWT_CYCCNT = 0xE0001004
DWT_CPICNT = 0xE0001008
DWT_EXCCNT = 0xE000100C
DWT_SLEEPCNT = 0xE0001010
DWT_LSUCNT = 0xE0001014
DWT_FOLDCNT = 0xE0001018


DWT_CYCCNT_Msk = 0x40000001 # counters mask values
DWT_ALL_Msk = 0x403F13F1


def set_counter():
    set_mem(DWT_CTRL, DWT_CYCCNT_Msk)


def reset_counter():
    set_mem(DWT_CYCCNT, 0)


# enables the other counters (other than the total cycles counter, e.g.SLEEPCNT..etc)
def set_all_counters():
    set_mem(DWT_CTRL, DWT_ALL_Msk)
    global counter
    counter = 0


def reset_all_counters():
    set_mem(DWT_CYCCNT, 0)
    set_mem(DWT_CPICNT, 0)
    set_mem(DWT_EXCCNT, 0)
    set_mem(DWT_SLEEPCNT, 0)
    set_mem(DWT_LSUCNT, 0)
    set_mem(DWT_FOLDCNT, 0)


'''
    Shortcuts for the functions above to ease debugging
'''

# These methods are same as above, just shorter to ease debugging
def sc():
    set_counter()


def rc():
    reset_counter()

def reset_cycc():
    sc()
    rc()

# short hand for set_all_counters()
def sac():
    set_all_counters()


# short hand to set and reset all counters
def rac():
    set_all_counters()
    reset_all_counters()


# short hand to print the value of all the counters
def pcntr():
    global DWT_CTRL
    global DWT_CYCCNT
    global DWT_CPICNT
    global DWT_EXCCNT
    global DWT_SLEEPCNT
    global DWT_LSUCNT
    global DWT_FOLDCNT

    #print("---------------------------------------------------------------")
    cntr_table = Texttable()
    cntr_table.add_rows([['COUNTER', 'VALUE'],
                         ['DWT_CYCCNT', get_mem(DWT_CYCCNT)],
                         ['DWT_CPICNT', get_mem(DWT_CPICNT)],
                         ['DWT_EXCCNT', get_mem(DWT_EXCCNT)],
                         ['DWT_SLEEPCNT', get_mem(DWT_SLEEPCNT)],
                         ['DWT_LSUCNT', get_mem(DWT_LSUCNT)],
                         ['DWT_FOLDCNT', get_mem(DWT_FOLDCNT)]])
    print(cntr_table.draw())
    print(long(get_mem(DWT_CYCCNT)))
    #print("---------------------------------------------------------------")


def get_bench_name():
    res = gdb.execute('info file', to_string=True)
    # Symbols... /.../../bench_name.elf".
    res = res.split("\n")[0].split("/")
    res = res[-1].split(".")[0]
    # to avoid any of mytool binary error, fix later [iot2-debug]
    if '--' in res:
        res = res.split('--')[0]
    return res


#######################################################################################################
#                                        Stack measurements                                           #
#######################################################################################################

MARK_VALUE = 0xdeadbeef

# fills RAM with 0xdeadbeaf
def set_stack(stack_start, stack_end):
    block_size = 32
    addr_step = 4   # cannot use block_size if compiled without symbols, so write by addr
    mark_list = []
    #for i in range(block_size):
    #    mark_list.append(str(MARK_VALUE))
    #mark_arr = "{" + ",".join(mark_list) + "}"
    for i in xrange(stack_start, stack_end, addr_step):
        gdb.execute("set *%s = %s" % (i, str(MARK_VALUE)))


# measure the used stack given the start and end addresses
def get_stack_highwater_mark(stack_start, stack_end):
    top_stack = stack_end
    watermark_end_addr = stack_start
    for i in xrange(stack_end, stack_start, -4):
        if get_value_from_addr(i) == MARK_VALUE:
            # check the next 2 addresses to make sure
            # we have found the highwater mark
            if (get_value_from_addr(i-4) == MARK_VALUE) and (get_value_from_addr(i-8) == MARK_VALUE):
                watermark_end_addr = i
                break
    used_stack = top_stack - watermark_end_addr
    return used_stack


# fills the memory with MARK_VALUE to enable measurement in the end
def fill_mem_section(start_addr, end_addr):
    print("-" * 80)
    print("[+] Setting up the stack measurement, this might take a while.....")
    print("-" * 80)
    addr_step = 4
    for i in xrange(start_addr, end_addr, addr_step):
        gdb.execute("set *%s = %s" % (i, str(MARK_VALUE)))
    return


# returns the used memory of the section given start and end
def get_used_mem_size(start_addr, end_addr):
    word_cntr = 0
    addr_step = 4
    for i in xrange(start_addr, end_addr, addr_step):
        if get_value_from_addr(i) != MARK_VALUE:
            word_cntr += 1
    mem_size = addr_step * word_cntr
    return mem_size


# collects the size of the stack and heap
def get_stack_heap_size(stack_top, stack_bottom, heap_start, heap_end):
    heap_size = 0 # (stack and heap are combined) get_used_mem_size(heap_start, heap_end)
    stack_size = get_used_mem_size(stack_bottom, stack_top)
    return heap_size, stack_size


def write_stack_usage(used_stack, results_file):
    print("-"*80)
    # check if we need to add os fixed overhead for timer, idle, and OS stacks
    if 'os_results' in results_file:
        used_stack += 768 + 512 + 4096
    print ("STACK = %d" % (used_stack))
    print("RESULTS_FILE = %s" % results_file)
    with open(results_file, 'r') as fd:
        json_results = json.load(fd)
    # add the stack usage results
    json_results["Stack_heap_usage"] = used_stack
    # write the updated results to the file
    with open(results_file, "w") as fd:
        json.dump(json_results, fd, sort_keys=True, indent=4, ensure_ascii=False)
    print("-"*80)


class EndBreakpoint(gdb.Breakpoint):
    def config(self, iterations, stack_top, bin_heap_start, heap_start, heap_size, results_file_path, file_ext):
        self.iterations = iterations
        self.stack_top = stack_top
        self.stack_bottom = bin_heap_start #self.stack_top - stack_size, replaced with bin_heap_start
        self.heap_start = heap_start
        self.heap_end = self.heap_start + heap_size
        self.results_file = results_file_path
        self.iteration_cntr = 1
        self.stack_usage_list = []

        # setup the stack for later measurement
        # [iot2-debug]: stack bottom here is actually the beginning of heap, we measure heap and stack
        #  together , fix later
        #fill_mem_section(self.stack_bottom, self.stack_top)


    def stop(self):

        if self.iteration_cntr < self.iterations:
            self.iteration_cntr += 1
            # added load to avoid differences and errors
            #load()
            mrh()
        else:
            # collect stack and heap usage, we actaully just write the result of both to stack_usage
            #heap_usage, stack_usage = get_stack_heap_size(self.stack_top, self.stack_bottom,
            #                                              self.heap_start, self.heap_end)
            #write_stack_usage(stack_usage, self.results_file)
            close_gdb()
        return False

