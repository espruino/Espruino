ESP_ZIP     = $(PROJ_NAME).tgz

CMAKEFILE = $(BINDIR)/main/CMakeLists.txt
# 'gen' has a relative path - get rid of it and add it manually
INCLUDE_WITHOUT_GEN = $(subst -Igen,,$(INCLUDE)) -I$(ROOT)/gen

$(CMAKEFILE):
	@mkdir -p $(BINDIR)/main # create directory if it doesn't exist
	@echo "MAKE CMAKEFILE"
	@echo "$(INCLUDE_WITHOUT_GEN)"
	@echo "idf_component_register(" > $(CMAKEFILE)
	@echo "						 SRCS" >> $(CMAKEFILE)
	@echo "						$(patsubst %,\"$(ROOT)/%\"\n						,$(SOURCES))" >> $(CMAKEFILE)
	@echo "						 INCLUDE_DIRS" >> $(CMAKEFILE)
	@echo "						$(patsubst -I%,\"%/\"\n						,$(INCLUDE_WITHOUT_GEN))" >> $(CMAKEFILE)
	@echo "						 )" >> $(CMAKEFILE)
	@echo "" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_TARGET} PUBLIC -DESP_IDF_VERSION_MAJOR=4)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_TARGET} PUBLIC $(DEFINES))" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_TARGET} PUBLIC -Og -fno-strict-aliasing -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -fgnu89-inline  -nostdlib -MMD -MP -Wno-enum-compare)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-pointer-sign)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-implicit-int)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-maybe-uninitialized)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-return-type)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-switch)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-unused-variable)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-unused-function)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-unused-but-set-variable)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-cast-function-type)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-format)" >> $(CMAKEFILE)


$(PROJ_NAME).bin: $(CMAKEFILE) $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h
	cp ${ROOT}/targets/esp32/IDF4/sdkconfig $(BINDIR)
	cp ${ROOT}/targets/esp32/IDF4/CMakeLists.txt $(BINDIR)
	cd $(BINDIR) && idf.py build

#$(ESP_ZIP): $(PROJ_NAME).bin

proj: $(PROJ_NAME).bin
#depend on $(ESP_ZIP)

#flash: $(PROJ_NAME).bin
