export CC=$(CCPREFIX)emcc
export LD=$(CCPREFIX)emcc

proj: 	$(PLATFORM_CONFIG_FILE) $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	@echo $($(quiet_)link)
	@$(call link)

