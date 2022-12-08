proj: 	$(PLATFORM_CONFIG_FILE) $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	@echo $($(quiet_)link)
	@$(call link)
	
sourcecode: $(SOURCES) $(PLATFORM_CONFIG_FILE)
	cat $(SOURCES) > gen/temp.c
	gcc $(DEFINES) $(INCLUDE) -E -P gen/temp.c -o gen/temp2.c
	cat targets/embed/embed_header.c gen/temp2.c > sourcecode.c
	rm gen/temp.c gen/temp2.c
