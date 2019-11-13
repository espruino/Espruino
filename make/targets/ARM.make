$(PROJ_NAME).elf: $(OBJS) $(LINKER_FILE)
	@echo $($(quiet_)link)
	@$(call link)

$(PROJ_NAME).lst : $(PROJ_NAME).elf
	@echo $($(quiet_)obj_dump)
	@$(call obj_dump)

$(PROJ_NAME).srec : $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,srec,srec)
	@$(call obj_to_bin,srec,srec)

$(PROJ_NAME).bin : $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,binary,bin)
	@$(call obj_to_bin,binary,bin)
	bash scripts/check_size.sh $(PROJ_NAME).bin
ifdef PAD_FOR_BOOTLOADER
	mv $(PROJ_NAME).bin $(PROJ_NAME).bin.unpadded
	tr "\000" "\377" < /dev/zero | dd bs=1 count=$(shell python scripts/get_board_info.py $(BOARD) "common.get_espruino_binary_address(board)") of=$(PROJ_NAME).bin
	cat $(PROJ_NAME).bin.unpadded >> $(PROJ_NAME).bin
endif

gdb:
	@echo "target extended-remote :4242" > gdbinit
	@echo "file $(PROJ_NAME).elf" >> gdbinit
	#echo "load" >> gdbinit
	@echo "break main" >> gdbinit
	@echo "break HardFault_Handler" >> gdbinit
	$(GDB) -x gdbinit
	rm gdbinit
