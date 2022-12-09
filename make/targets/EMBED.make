proj: 	$(PLATFORM_CONFIG_FILE) $(PROJ_NAME)
	
$(PROJ_NAME): $(SOURCES) $(PLATFORM_CONFIG_FILE)
	cat $(SOURCES) > $(PROJ_NAME)
	gcc $(DEFINES) $(INCLUDE) -E -P $(PROJ_NAME) -o gen/temp.c
	cat targets/embed/embed_header.c gen/temp.c > $(PROJ_NAME)
	rm gen/temp.c
	@echo ========================================
	@echo Created $(PROJ_NAME)
	@echo Test with "gcc $(PROJ_NAME) -lm"
