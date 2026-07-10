CMAKEFILE = $(ROOT)/gen/CMakeLists.txt
# 'gen' has a relative path - get rid of it and add it manually
INCLUDE_WITHOUT_GEN = $(subst -Igen,,$(INCLUDE)) -I$(ROOT)/gen

cmake: $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h $(PININFOFILE).c $(WRAPPERFILE)
	@echo "MAKE CMAKEFILE"
	@echo "$(INCLUDE_WITHOUT_GEN)"
	@echo "target_sources(app PRIVATE" > $(CMAKEFILE)
	@echo "					$(patsubst %,\"$(ROOT)/%\"\n					,$(SOURCES))" >> $(CMAKEFILE)
	@echo ")" >> $(CMAKEFILE)
	@echo "target_include_directories(app PRIVATE" >> $(CMAKEFILE)
	@echo "					$(patsubst -I%,\"%/\"\n					,$(INCLUDE_WITHOUT_GEN))" >> $(CMAKEFILE)
	@echo ")" >> $(CMAKEFILE)
	@echo "" >> $(CMAKEFILE)
	@echo "target_compile_options(app PRIVATE -DZEPHYR=1)" >> $(CMAKEFILE)
	@echo "target_compile_options(app PRIVATE $(DEFINES))" >> $(CMAKEFILE)
	@echo ""
	@echo "========================================================================="
	@echo "  Now build the project in targets/zephyr with NRF Connect SDK"
	@echo "========================================================================="