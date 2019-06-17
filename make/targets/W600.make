$(PROJ_NAME).elf: $(OBJS) $(LINKER_FILE)
	@echo $($(quiet_)link)
	@$(call link)

$(PROJ_NAME).bin : $(PROJ_NAME).elf
	@echo $(call $(quiet_)obj_to_bin,binary,bin)
	@$(call obj_to_bin,binary,bin)
	@cp $(ROOT)/$(PROJ_NAME).bin $(ROOT)/$(PROJ_NAME).bin.bk
	@gzip -fv $(ROOT)/$(PROJ_NAME).bin
	@mv $(ROOT)/$(PROJ_NAME).bin.bk $(ROOT)/$(PROJ_NAME).bin
	@$(SDK_ROOT)/tools/makeimg  $(ROOT)/$(PROJ_NAME).bin "$(ROOT)/$(PROJ_NAME).img" 0 0 "$(SDK_ROOT)/bin/version.txt" 90000 10100
	@$(SDK_ROOT)/tools/makeimg  $(ROOT)/$(PROJ_NAME).bin.gz "$(ROOT)/$(PROJ_NAME)_gz.img" 0 1 "$(SDK_ROOT)/bin/version.txt" 90000 10100 $(ROOT)/$(PROJ_NAME).bin
	@$(SDK_ROOT)/tools/makeimg  $(ROOT)/$(PROJ_NAME).bin "$(ROOT)/$(PROJ_NAME)_sec.img" 0 0 "$(SDK_ROOT)/bin/version.txt" 90000 10100
	bash scripts/check_size.sh $(PROJ_NAME).bin
	@$(SDK_ROOT)/tools/makeimg_all "$(SDK_ROOT)/bin/secboot.img" "$(ROOT)/$(PROJ_NAME).img" "$(ROOT)/$(PROJ_NAME).fls"
	$(Q)rm -f $(PROJ_NAME)_sec.img
	$(Q)rm -f $(PROJ_NAME).bin.gz
	$(Q)rm -f $(PROJ_NAME).img
	$(Q)rm -f $(PROJ_NAME).bin
	$(Q)rm -f $(PROJ_NAME).elf
	@echo "Build finish !!!"
proj:$(PROJ_NAME).bin