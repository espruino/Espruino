PROJ_HEADER_NAME = $(PROJ_NAME:.c=.h)

proj: 	$(PLATFORM_CONFIG_FILE) $(PROJ_NAME)
	
$(PROJ_NAME): $(SOURCES) $(PLATFORM_CONFIG_FILE)
	gcc $(DEFINES) $(INCLUDE) -E -P targets/embed/embed.h -o gen/temp.h
	cat targets/embed/embed_header.h gen/temp.h targets/embed/embed_footer.h > $(PROJ_HEADER_NAME)
	cat $(SOURCES) > $(PROJ_NAME)
	gcc $(DEFINES) $(INCLUDE) -E -P $(PROJ_NAME) -o gen/temp.c
	cat targets/embed/embed_header.c gen/temp.c > $(PROJ_NAME)
	rm gen/temp.c gen/temp.h
	@echo ========================================
	@echo Created $(PROJ_HEADER_NAME)
	@echo Created $(PROJ_NAME)	
	@echo Test with:  "gcc targets/embed/test.c $(PROJ_NAME) -lm"
