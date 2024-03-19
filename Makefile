RISCV_XLEN ?= 64
RISCV_LIB  ?= elf
BMARK ?= disparity
#
DSET ?= sim

TARGET=riscv${RISCV_XLEN}-unknown-${RISCV_LIB}
#-----------------------------------------------------------
GCC		= $(TARGET)-gcc
OBJCOPY	= $(TARGET)-objcopy
OBJDUMP	= $(TARGET)-objdump
AR		= $(TARGET)-ar

GCCVER 	= $(shell $(GCC) --version | grep gcc | cut -d" " -f9)

#64-bit
#RISCV ?= ~/riscv_fpga  
#32-bit
#RISCV ?= ~/riscv
RISCV_CCPATH = ${CCPATH}

BUILD_DIR       = build
FREERTOS_SOURCE_DIR = $(abspath ./FreeRTOS/Source)
DEMO_SOURCE_DIR = $(abspath ./FreeRTOS/Demo/Common/Minimal)

INCLUDES = \
	-I. \
	-I$(FREERTOS_SOURCE_DIR)/include \
	-I$(FREERTOS_SOURCE_DIR)/../Demo/Common/include \
	-I$(FREERTOS_SOURCE_DIR)/portable/GCC/RISC-V \
	-I./uart \
	$(addprefix -I, $(INC_DIRS))
#-----------------------------------------------------------


# cva6
CPPFLAGS = 

CFLAGS = \
	-Wall \
	$(INCLUDES) \
	-fomit-frame-pointer \
	-fno-strict-aliasing \
	-fno-builtin-printf \
	-D__gracefulExit \
	-Driscv \
	-mcmodel=medany \
	-static \
	-DRISCV \
	-D___$(BMARK)___\
	-I $(FREERTOS_SOURCE_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions


LDFLAGS	 = -T link.ld \
			-nostartfiles \
			-static \
			-fno-builtin-printf \
			-Xlinker --defsym=__stack_size=300 
			#-Xlinker -Map=RTOSDemo.map # output file

LIBS	 = -L$(RISCV_CCPATH)/lib/gcc/$(TARGET)/$(GCCVER) \
		   -L$(RISCV_CCPATH)/$(TARGET)/lib \
		   -lc \
		   -lgcc


DEBUG = 0
ifeq ($(DEBUG), 1)
    CFLAGS += -O0 -ggdb3
else
    CFLAGS += -O2
endif

#-----------------------------------------------------------

FREERTOS_SRC = \
	$(FREERTOS_SOURCE_DIR)/croutine.c \
	$(FREERTOS_SOURCE_DIR)/event_groups.c \
	$(FREERTOS_SOURCE_DIR)/list.c \
	$(FREERTOS_SOURCE_DIR)/queue.c \
	$(FREERTOS_SOURCE_DIR)/stream_buffer.c \
	$(FREERTOS_SOURCE_DIR)/tasks.c \
	$(FREERTOS_SOURCE_DIR)/timers.c \
	$(FREERTOS_SOURCE_DIR)/portable/MemMang/heap_4.c \
	$(FREERTOS_SOURCE_DIR)/portable/GCC/RISC-V/port.c


DEMO_SRC =  \
	$(DEMO_SOURCE_DIR)/blocktim.c \
	$(DEMO_SOURCE_DIR)/countsem.c \
	$(DEMO_SOURCE_DIR)/recmutex.c \
	$(DEMO_SOURCE_DIR)/death.c \
	$(DEMO_SOURCE_DIR)/TimerDemo.c \
	$(DEMO_SOURCE_DIR)/EventGroupsDemo.c \
	$(DEMO_SOURCE_DIR)/TaskNotify.c \
	$(DEMO_SOURCE_DIR)/GenQTest.c \
	$(DEMO_SOURCE_DIR)/BlockQ.c \
	$(DEMO_SOURCE_DIR)/PollQ.c \
	$(DEMO_SOURCE_DIR)/QPeek.c \
	$(DEMO_SOURCE_DIR)/QueueOverwrite.c \
	$(DEMO_SOURCE_DIR)/AbortDelay.c \
	$(DEMO_SOURCE_DIR)/semtest.c \
	$(DEMO_SOURCE_DIR)/StreamBufferDemo.c \
	$(DEMO_SOURCE_DIR)/StreamBufferInterrupt.c \
	$(DEMO_SOURCE_DIR)/dynamic.c \
	$(DEMO_SOURCE_DIR)/integer.c \
	$(DEMO_SOURCE_DIR)/MessageBufferDemo.c 


ROOT_DIR:=$(realpath .)
freertos_fat_dir:=$(ROOT_DIR)/FreeRTOS-FAT
freertos_ramdisk_dir:=$(freertos_fat_dir)/portable/common
SRC_DIRS+=$(freertos_fat_dir) $(freertos_ramdisk_dir)
INC_DIRS+=$(freertos_fat_dir)/include
INC_DIRS+=$(freertos_fat_dir)/portable/common
FREERTOS_SRC+=$(wildcard $(freertos_fat_dir)/*.c)
FREERTOS_SRC+=$(freertos_fat_dir)/portable/common/ff_ramdisk.c


# source for sdvb benchmarks
# sdvb_disp_dir:=$(ROOT_DIR)/sdvb/disp
# FREERTOS_SRC+=$(wildcard $(sdvb_disp_dir)/*.c)
# INC_DIRS+=$(sdvb_disp_dir)


#source for all vision benchmarks
sdvb_bmark_dir:=$(ROOT_DIR)/vision/benchmarks/$(BMARK)/src/c
FREERTOS_SRC+=$(wildcard $(sdvb_bmark_dir)/*.c)
INC_DIRS+=$(sdvb_bmark_dir)

sdvb_common_dir:=$(ROOT_DIR)/vision/common/c
FREERTOS_SRC+=$(wildcard $(sdvb_common_dir)/*.c)
INC_DIRS+=$(sdvb_common_dir)



ASM_SRC = start.S \
		  vector.S \
		  RegTest.S \
	      $(FREERTOS_SOURCE_DIR)/portable/GCC/RISC-V/portASM.S 

APP_SRC = \
	main.c \
	RegTests.c \
	riscv-virt.c \
	uart/uart.c
	

OBJS = $(FREERTOS_SRC:%.c=$(BUILD_DIR)/%.o) \
	   $(DEMO_SRC:%.c=$(BUILD_DIR)/%.o) \
	   $(ASM_SRC:%.S=$(BUILD_DIR)/%.o) \
	   $(APP_SRC:%.c=$(BUILD_DIR)/%.o)

DEPS = $(FREERTOS_SRC:%.c=$(BUILD_DIR)/%.d) \
		$(DEMO_SRC:%.c=$(BUILD_DIR)/%.d) \
		$(ASM_SRC:%.S=$(BUILD_DIR)/%.d) \
		$(APP_SRC:%.c=$(BUILD_DIR)/%.d)

$(BUILD_DIR)/RTOSDemo.elf: $(OBJS) link.ld Makefile
	$(GCC) $(LDFLAGS) $(LIBS) $(OBJS) -o $@ -lc -lm

$(BUILD_DIR)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(GCC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile
	@mkdir -p $(@D)
	$(GCC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

dis:
	$(TARGET)-objdump -d $(BUILD_DIR)/RTOSDemo.elf > RTOSDemo.asm

-include $(DEPS)

