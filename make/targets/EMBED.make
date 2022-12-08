proj: 	$(PLATFORM_CONFIG_FILE) $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	@echo $($(quiet_)link)
	@$(call link)
	
sourcecode: $(SOURCES) $(PLATFORM_CONFIG_FILE)
	cat $(SOURCES) > gen/out.c
	@echo gcc $(INCLUDE) -E gen/out.c	
	gcc $(DEFINES) $(INCLUDE) -E -P gen/out.c -o sourcecode.c
#	rm gen/out.c
