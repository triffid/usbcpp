#
#
#

PROJECT  = usbcpp

CSRC     = $(wildcard *.c)
CXXSRC   = $(wildcard *.cpp) $(wildcard usb/*.cpp)
ASRC     = $(wildcard *.S)

SUBDIRS  = Drivers Core

INC      = . $(shell find */ -type d)

LIBRARIES =

OUTDIR   = build

OSRC     =

NXPSRC   = $(shell find CMSISv2p00_LPC17xx/ LPC17xxLib/ -name '*.c')
NXPO     = $(patsubst %.c,$(OUTDIR)/%.o,$(notdir $(NXPSRC))) $(OUTDIR)/system_LPC17xx.o

CHIP     = lpc1758
MCU      = cortex-m3

ARCH     = arm-none-eabi
PREFIX   = $(ARCH)-

CC       = $(PREFIX)gcc
CXX      = $(PREFIX)g++
OBJCOPY  = $(PREFIX)objcopy
OBJDUMP  = $(PREFIX)objdump
AR       = $(PREFIX)ar
SIZE     = $(PREFIX)size

MKDIR    = mkdir
RMDIR    = rmdir
RM       = rm -f

OPTIMIZE = s

#DEBUG_MESSAGES
CDEFS    = MAX_URI_LENGTH=512 __LPC17XX__

FLAGS    = -O$(OPTIMIZE) -mcpu=$(MCU) -mthumb -mthumb-interwork -mlong-calls -ffunction-sections -fdata-sections -Wall -g -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
FLAGS   += $(patsubst %,-I%,$(INC))
FLAGS   += $(patsubst %,-D%,$(CDEFS))
CFLAGS   = $(FLAGS) -std=gnu99 -pipe
ASFLAGS  = $(FLAGS)
CXXFLAGS = $(FLAGS) -fno-rtti -fno-exceptions -std=gnu++0x

LDFLAGS  = $(FLAGS) -Wl,--as-needed,--gc-sections,-e,__cs3_reset_cortex_m,-T,lpc1758.ld
LDFLAGS += $(patsubst %,-L%,$(LIBRARIES)) -lc -lstdc++

OBJ      = $(patsubst %,$(OUTDIR)/%,$(notdir $(CSRC:.c=.o) $(CXXSRC:.cpp=.o) $(ASRC:.S=.o)))

VPATH    = . $(patsubst %/inc,%/src,$(INC)) $(dir $(NXPSRC)) $(dir $(USBSRC)) $(dir $(UIPSRC)) $(dir $(LWIPSRC))

.PHONY: all clean program upload size

.PRECIOUS: $(OBJ)

all: $(OUTDIR) $(OUTDIR)/$(PROJECT).elf $(OUTDIR)/$(PROJECT).bin $(OUTDIR)/$(PROJECT).hex size

clean:
	$(RM) $(OBJ) $(OUTDIR)/$(PROJECT).bin $(OUTDIR)/$(PROJECT).hex $(OUTDIR)/$(PROJECT).elf $(NXPO)
	$(RMDIR) $(OUTDIR)

program: $(OUTDIR)/$(PROJECT).bin
	mount /mnt/r2c2
	cp $< /mnt/r2c2/firmware.bin
	eject /mnt/r2c2
upload: program

size: $(OUTDIR)/$(PROJECT).elf
	@$(SIZE) $<

$(OUTDIR):
	@$(MKDIR) $(OUTDIR)

$(OUTDIR)/%.bin: $(OUTDIR)/%.elf
	@echo "  COPY  " $@
	@$(OBJCOPY) -O binary $< $@

$(OUTDIR)/%.hex: $(OUTDIR)/%.elf
	@echo "  COPY  " $@
	@$(OBJCOPY) -O ihex $< $@

$(OUTDIR)/%.sym: $(OUTDIR)/%.elf
	@echo "  SYM   " $@
	@$(OBJDUMP) -t $< | perl -ne 'BEGIN { printf "%6s  %-40s %s\n", "ADDR","NAME","SIZE"; } /([0-9a-f]+)\s+(\w+)\s+O\s+\.(bss|data)\s+([0-9a-f]+)\s+(\w+)/ && printf "0x%04x  %-40s +%d\n", eval("0x$$1") & 0xFFFF, $$5, eval("0x$$4")' | sort -k1 > $@

$(OUTDIR)/%.elf: $(OBJ) $(OUTDIR)/nxp.ar
	@echo "  LINK  " $@
	@$(CXX) $(OSRC) -Wl,-Map=$(@:.elf=.map) -o $@ $^ $(LDFLAGS)

$(OUTDIR)/%.o: %.c
	@echo "  CC    " $@
	@$(CC) $(CFLAGS) -Wa,-adhlns=$(@:.o=.lst) -c -o $@ $<

$(OUTDIR)/%.o: %.cpp
	@echo "  CXX   " $@
	@$(CXX) $(CXXFLAGS) -Wa,-adhlns=$(@:.o=.lst) -c -o $@ $<

$(OUTDIR)/%.o: %.S
	@echo "  AS    " $@
	@$(CC) $(ASFLAGS) -Wa,-adhlns=$(@:.o=.lst) -c -o $@ $<

$(OUTDIR)/nxp.ar: $(NXPO)
	@echo "  AR    " $@
	@$(AR) ru $@ $^
