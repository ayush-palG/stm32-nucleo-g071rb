TARGET  = main
CC      = arm-none-eabi-gcc
# OBJCOPY = arm-none-eabi-objcopy

BUILD_DIR   = build/

CPU_FLAGS = -mcpu=cortex-m0plus -mthumb
CFLAGS    = $(CPU_FLAGS) -Wall -Wextra -g3 -Og
LDFLAGS   = -T linker.ld -nostdlib

OPENOCD_CMD = program $(BUILD_DIR)$(TARGET).elf verify reset exit

all: $(BUILD_DIR)$(TARGET).elf

$(BUILD_DIR)%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $^ -o $@

$(BUILD_DIR)$(TARGET).elf: $(BUILD_DIR)$(TARGET).o $(BUILD_DIR)startup.o
	@mkdir -p $(BUILD_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

flash: $(BUILD_DIR)$(TARGET).elf
	openocd -f openocd.cfg -c "$(OPENOCD_CMD)"

clean:
	rm -rf $(BUILD_DIR)
