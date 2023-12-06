# makefile for compiling and burning AVR uCs
# sudo apt-get install gcc-avr avr-libc avrdude

MCU=attiny44a
MCU_USBASP=t44
CLOCK=1000000

###

DIR_BUILD        = ./build
DIR_BIN          = ${DIR_BUILD}/bin
DIR_OUTPUT       = ${DIR_BUILD}/output
DIR_SRC          = ./src

OUTPUT_FILE_PATH = ${DIR_BUILD}/main.elf

C_SOURCES        = $(shell find ${DIR_SRC} -name "*.cpp")
OBJECTS          = $(subst ${DIR_SRC},${DIR_BIN},${C_SOURCES:.cpp=.o})

GIT_DESCRIBE     = $(shell git describe --always --dirty)

###

F_MCU   = -mmcu=${MCU}
F_CPU   = -DF_CPU=${CLOCK}UL
F_OPT   = -Os -gdwarf-2
F_TYPES = -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -fshort-enums
F_DEP   = -MMD -MP -MF"$(@:%.o=%.d)"

DEFINES = '-DVERSION_TXT="${GIT_DESCRIBE}"' -DPLATFORM=${PLATFORM}

C_FLAGS = -std=c++11 -Wall -Werror ${DEFINES} ${F_OPT} ${F_TYPES} ${F_MCU} ${F_CPU} -c ${F_DEP}
L_FLAGS = -Wl,-Map="${DIR_BUILD}/main.map" -Wl,--start-group  -Wl,--end-group -Wl,--gc-sections

# ---------------------------------------------------------------------------------------------------------------------

all: ${OUTPUT_FILE_PATH}

${DIR_BIN}:
	mkdir -p $@

${DIR_OUTPUT}:
	mkdir -p $@

${DIR_BIN}/%.o: ${DIR_SRC}/%.cpp Makefile | ${DIR_BIN}
	avr-gcc ${C_FLAGS}  $< -o $@

${OUTPUT_FILE_PATH}: ${OBJECTS} | ${DIR_OUTPUT}
	@echo Building target: $@
	avr-gcc -o${OUTPUT_FILE_PATH} ${OBJECTS} ${F_MCU} ${L_FLAGS}
	@echo Finished building target: $@
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "${DIR_BUILD}/main.elf" "${DIR_BUILD}/main.hex"
	avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "${DIR_BUILD}/main.elf" "${DIR_BUILD}/main.eep" || exit 0
	avr-objdump -h -S "${DIR_BUILD}/main.elf" > "${DIR_BUILD}/main.lss"
	avr-objcopy -O srec -R .eeprom -R .fuse -R .lock -R .signature  "${DIR_BUILD}/main.elf" "${DIR_BUILD}/main.srec"
	avr-size "${DIR_BUILD}/main.elf"
	cp ${DIR_BUILD}/main.hex ${DIR_OUTPUT}/icicle-${GIT_DESCRIBE}.hex
	@echo
	@echo "Version: ${GIT_DESCRIBE}"

clean:
	rm -Rf  ${DIR_BIN} ${DIR_BUILD}/main.*

clean-deep: clean
	rm -Rf  ${DIR_BUILD}

# ---------------------------------------------------------------------------------------------------------------------

.PHONY: format-all

format-all:
	find ./src -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i

flash: ${OUTPUT_FILE_PATH}
	avrdude -P usb -c usbasp -F -p ${MCU_USBASP} -U flash:w:${DIR_BUILD}/main.hex:i

versioninfo:
	@echo ${GIT_DESCRIBE}

print-% : ; @echo $* = ${$*}

-include $(shell find ${BUILD_DIR} -name "*.d")
