BUILD_DIR=build
# Sucht automatisch die erste .uf2 Datei im Build-Ordner
UF2_FILE=$(shell find $(BUILD_DIR) -maxdepth 1 -name "*.uf2" 2>/dev/null)

default:
	$(MAKE) -C $(BUILD_DIR) -j4

all:
	# Falls pico_sdk_import.cmake fehlt, kopieren
	[ -f pico_sdk_import.cmake ] || cp $(PICO_SDK_PATH)/external/pico_sdk_import.cmake .
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make -j4
	# Symlink oder Kopie für Neovim/clangd
	cp $(BUILD_DIR)/compile_commands.json .

flash:
	@if [ -z "$(UF2_FILE)" ]; then echo "Fehler: Keine .uf2 Datei gefunden. Erst 'make all' ausführen!"; exit 1; fi
	picotool load $(UF2_FILE) -f

clean:
	rm -rf $(BUILD_DIR)
	rm -f compile_commands.json

.PHONY: default all flash clean
