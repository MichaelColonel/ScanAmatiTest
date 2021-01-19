################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
scanner_firmware_ver2.lss \

FLASH_IMAGE += \
scanner_firmware_ver2.hex \

EEPROM_IMAGE += \
scanner_firmware_ver2.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: scanner_firmware_ver2.elf secondary-outputs

# Tool invocations
scanner_firmware_ver2.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,scanner_firmware_ver2.map -mmcu=atmega64 -o "scanner_firmware_ver2.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

scanner_firmware_ver2.lss: scanner_firmware_ver2.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S scanner_firmware_ver2.elf  >"scanner_firmware_ver2.lss"
	@echo 'Finished building: $@'
	@echo ' '

scanner_firmware_ver2.hex: scanner_firmware_ver2.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex scanner_firmware_ver2.elf  "scanner_firmware_ver2.hex"
	@echo 'Finished building: $@'
	@echo ' '

scanner_firmware_ver2.eep: scanner_firmware_ver2.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex scanner_firmware_ver2.elf  "scanner_firmware_ver2.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: scanner_firmware_ver2.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=berkeley -t scanner_firmware_ver2.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) scanner_firmware_ver2.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
