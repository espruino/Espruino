$(PROJ_NAME).elf: $(OBJS) $(LINKER_FILE)
	@echo $($(quiet_)link)
	@$(call link)

$(PROJ_NAME).lst : $(PROJ_NAME).elf
	@echo $($(quiet_)obj_dump)
	@$(call obj_dump)

$(PROJ_NAME).hex: $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,ihex,hex)
	@$(call obj_to_bin,ihex,hex)
ifdef SOFTDEVICE # Shouldn't do this when we want to be able to perform DFU OTA!
 ifdef USE_BOOTLOADER
  ifdef DFU_UPDATE_BUILD
	@echo Not merging softdevice or bootloader with application
	# nrfutil  pkg generate --help
	nrfutil pkg generate $(PROJ_NAME).zip --application $(PROJ_NAME).hex $(DFU_SETTINGS) --key-file $(DFU_PRIVATE_KEY)
  else
  ifdef BOOTLOADER
	@echo Not merging anything with bootloader
  else
	@echo Merging SoftDevice and Bootloader
	# We can build a DFU settings file we can merge in...
	# nrfutil settings generate --family NRF52 --application $(PROJ_NAME).hex --application-version 0xff --bootloader-version 0xff --bl-settings-version 1 dfu_settings.hex
	@echo FIXME - had to set --overlap=replace
	python scripts/hexmerge.py --overlap=replace $(SOFTDEVICE) $(NRF_BOOTLOADER) $(PROJ_NAME).hex -o tmp.hex
	mv tmp.hex $(PROJ_NAME).hex
  endif
  endif
 else
	@echo Merging SoftDevice
	python scripts/hexmerge.py $(SOFTDEVICE) $(PROJ_NAME).hex -o tmp.hex
	mv tmp.hex $(PROJ_NAME).hex
 endif # USE_BOOTLOADER
endif # SOFTDEVICE

$(PROJ_NAME).srec : $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,srec,srec)
	@$(call obj_to_bin,srec,srec)

$(PROJ_NAME).bin : $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,binary,bin)
	@$(call obj_to_bin,binary,bin)
ifndef TRAVIS
	bash scripts/check_size.sh $(PROJ_NAME).bin
endif
ifdef PAD_FOR_BOOTLOADER
	mv $(PROJ_NAME).bin $(PROJ_NAME).bin.unpadded
	tr "\000" "\377" < /dev/zero | dd bs=1 count=$(shell python scripts/get_board_info.py $(BOARD) "common.get_espruino_binary_address(board)") of=$(PROJ_NAME).bin
	cat $(PROJ_NAME).bin.unpadded >> $(PROJ_NAME).bin
endif

ifdef NRF5X
proj: $(PROJ_NAME).lst $(PROJ_NAME).hex
else
proj: $(PROJ_NAME).lst $(PROJ_NAME).bin $(PROJ_NAME).hex
endif

flash: all
ifdef USE_DFU
	sudo dfu-util -a 0 -s 0x08000000 -D $(PROJ_NAME).bin
else ifdef OLIMEXINO_STM32_BOOTLOADER
	@echo Olimexino Serial bootloader
	dfu-util -a1 -d 0x1EAF:0x0003 -D $(PROJ_NAME).bin
else ifdef NUCLEO
	if [ -d "/media/$(USER)/NUCLEO" ]; then cp $(PROJ_NAME).bin /media/$(USER)/NUCLEO;sync; fi
	if [ -d "/media/NUCLEO" ]; then cp $(PROJ_NAME).bin /media/NUCLEO;sync; fi
else ifdef MICROBIT
	if [ -d "/media/$(USER)/MICROBIT" ]; then cp $(PROJ_NAME).hex /media/$(USER)/MICROBIT;sync; fi
	if [ -d "/media/MICROBIT" ]; then cp $(PROJ_NAME).hex /media/MICROBIT;sync; fi
else ifdef NRF5X
	if type nrfjprog 2>/dev/null; then nrfjprog --family $(FAMILY) --clockspeed 50000 --program $(PROJ_NAME).hex --chiperase --reset; \
	elif [ -d "/media/$(USER)/JLINK" ]; then cp $(PROJ_NAME).hex /media/$(USER)/JLINK;sync; \
	elif [ -d "/media/JLINK" ]; then cp $(PROJ_NAME).hex /media/JLINK;sync; fi
else
	@echo ST-LINK flash
	st-flash --reset write $(PROJ_NAME).bin $(BASEADDRESS)
endif

serialflash: all
	@echo STM32 inbuilt serial bootloader, set BOOT0=1, BOOT1=0
	python scripts/stm32loader.py -b 460800 -a $(BASEADDRESS) -ew $(STM32LOADER_FLAGS) $(PROJ_NAME).bin
#	python scripts/stm32loader.py -b 460800 -a $(BASEADDRESS) -ewv $(STM32LOADER_FLAGS) $(PROJ_NAME).bin

gdb:
	@echo "target extended-remote :4242" > gdbinit
	@echo "file $(PROJ_NAME).elf" >> gdbinit
	#echo "load" >> gdbinit
	@echo "break main" >> gdbinit
	@echo "break HardFault_Handler" >> gdbinit
	$(GDB) -x gdbinit
	rm gdbinit
