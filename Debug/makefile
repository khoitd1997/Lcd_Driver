################################################################################
# Automatically-generated file. Do not edit!
################################################################################

CG_TOOL_ROOT := /home/kd/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.3.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./lcd_driver.obj" \
"./lcd_utils.obj" \
"./main.obj" \
"./tm4c123gh6pm_startup_ccs.obj" \
"./Tivaware_Dep/driverlib/epi_workaround_ccs.obj" \
"./Tivaware_Dep/drivers/buttons.obj" \
"./Tivaware_Dep/drivers/rgb.obj" \
"./Tivaware_Dep/utils/softuart.obj" \
"./Tivaware_Dep/utils/uartstdio.obj" \
"./Tivaware_Dep/utils/ustdlib.obj" \
"./general_timer/general_timer.obj" \
"../tm4c123gh6pm.cmd" \
"../Tivaware_Dep/IQmath/ccs/IQmathLib/Debug/IQmathLib.lib" \
"../Tivaware_Dep/driverlib/ccs/Debug/driverlib.lib" \
"../Tivaware_Dep/sensorlib/ccs/Debug/sensorlib.lib" \
$(GEN_CMDS__FLAG) \
-llibc.a \
-l"/home/kd/Lcd_Driver/Tivaware_Dep/sensorlib/ccs/Debug/sensorlib.lib" \
-l"/home/kd/Lcd_Driver/Tivaware_Dep/driverlib/ccs/Debug/driverlib.lib" \

-include ../makefile.init

RM := rm -rf
RMDIR := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Tivaware_Dep/IQmath/ccs/IQmathLib/Debug/subdir_vars.mk
-include Tivaware_Dep/driverlib/ccs/Debug/subdir_vars.mk
-include Tivaware_Dep/driverlib/subdir_vars.mk
-include Tivaware_Dep/drivers/subdir_vars.mk
-include Tivaware_Dep/sensorlib/ccs/Debug/subdir_vars.mk
-include Tivaware_Dep/utils/subdir_vars.mk
-include general_timer/subdir_vars.mk
-include subdir_rules.mk
-include Tivaware_Dep/IQmath/ccs/IQmathLib/Debug/subdir_rules.mk
-include Tivaware_Dep/driverlib/ccs/Debug/subdir_rules.mk
-include Tivaware_Dep/driverlib/subdir_rules.mk
-include Tivaware_Dep/drivers/subdir_rules.mk
-include Tivaware_Dep/sensorlib/ccs/Debug/subdir_rules.mk
-include Tivaware_Dep/utils/subdir_rules.mk
-include general_timer/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
lcd_tiva.out \

EXE_OUTPUTS__QUOTED += \
"lcd_tiva.out" \

BIN_OUTPUTS += \
lcd_tiva.hex \

BIN_OUTPUTS__QUOTED += \
"lcd_tiva.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "lcd_tiva.out"

# Tool invocations
lcd_tiva.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: ARM Linker'
	"/home/kd/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=ccs="ccs" --define=TARGET_IS_TM4C123_RB1 --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"lcd_tiva.map" --heap_size=0 --stack_size=2048 -i"/home/kd/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.3.LTS/lib" -i"/home/kd/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.3.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="lcd_tiva_linkInfo.xml" --rom_model -o "lcd_tiva.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

lcd_tiva.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: ARM Hex Utility'
	"/home/kd/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.3.LTS/bin/armhex"  -o "lcd_tiva.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "lcd_driver.obj" "lcd_utils.obj" "main.obj" "tm4c123gh6pm_startup_ccs.obj" "Tivaware_Dep/driverlib/epi_workaround_ccs.obj" "Tivaware_Dep/drivers/buttons.obj" "Tivaware_Dep/drivers/rgb.obj" "Tivaware_Dep/utils/softuart.obj" "Tivaware_Dep/utils/uartstdio.obj" "Tivaware_Dep/utils/ustdlib.obj" "general_timer/general_timer.obj" 
	-$(RM) "tm4c123gh6pm_startup_ccs.d" "Tivaware_Dep/drivers/buttons.d" "Tivaware_Dep/drivers/rgb.d" "Tivaware_Dep/utils/softuart.d" "Tivaware_Dep/utils/uartstdio.d" "Tivaware_Dep/utils/ustdlib.d" 
	-$(RM) "lcd_driver.d" "lcd_utils.d" "main.d" "general_timer/general_timer.d" 
	-$(RM) "Tivaware_Dep/driverlib/epi_workaround_ccs.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

