PROJ_HEADER_NAME = $(PROJ_NAME:.c=.h)

proj: 	$(PLATFORM_CONFIG_FILE) $(PROJ_NAME)
	
$(PROJ_NAME): $(SOURCES) $(PLATFORM_CONFIG_FILE)
	gcc $(DEFINES) $(INCLUDE) -E -P targets/embed/embed.h -o $(OBJDIR)/temp.h
	cat targets/embed/embed_header.h $(OBJDIR)/temp.h targets/embed/embed_footer.h > $(PROJ_HEADER_NAME)
	cat $(SOURCES) > $(PROJ_NAME)
	gcc $(DEFINES) $(INCLUDE) -E -P $(PROJ_NAME) -o $(OBJDIR)/temp.c
	cat targets/embed/embed_header.c $(OBJDIR)/temp.c > $(PROJ_NAME)
	cp targets/embed/embed_utils.h $(BINDIR)/espruino_embedded_utils.h
	rm $(OBJDIR)/temp.c $(OBJDIR)/temp.h
	@echo ========================================
	@echo Created $(PROJ_HEADER_NAME)
	@echo Created $(PROJ_NAME)	
	@echo Created $(BINDIR)/espruino_embedded_utils.h
	@echo ========================================	
	@echo Test with:  "gcc targets/embed/test.c $(PROJ_NAME) -Isrc -lm -m32"
	@echo NOTE: This build is for 32 bit targets only
