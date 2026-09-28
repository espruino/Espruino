var printCompleteSpecs = function() {
  var env = process.env;
  var mem = process.memory();
  var esp = ESP32.getState();
  print("\n[ SYSTEM INFO ]");
  print("  Board Target     : " + env.BOARD);
  print("  SoC Model        : " + (esp.model ? esp.model.toUpperCase() : "ESP32"));
  print("  CPU Core Count   : " + (esp.cores || 2) + " Cores");
  print("  CPU frequence    : " + ( Element.getClock() / 1E6) + " MHz");
  print("  Silicon Revision : v" + (esp.revision !== undefined ? esp.revision : "N/A"));
  print("  Espruino Version : " + env.VERSION);
  print("  IDF SDK Version  : " + esp.sdkVersion);
  print("  Unique Chip ID   : " + env.SERIAL);
  print("\n[ SILICON & MEMORY DEEP-DIVE ]");
  print("  External PSRAM   : " + ((esp.psramSize && esp.psramSize > 0) ? Math.round(esp.psramSize / 1024 / 1024) + " MB Detected" : "None Found"));
  print("  Internal SRAM    : " + (env.RAM / 1024) + " KB total static layout");
  print("  OS Free Heap     : " + (esp.freeHeap / 1024).toFixed(1) + " KB");
  print("  Min Free Heap    : " + (esp.minHeap / 1024).toFixed(1) + " KB");
  print("\n[ PHYSICAL FLASH STORAGE ]");
  if (esp.flashSize) {
    print("  Total Flash Size : " + (esp.flashSize / 1024 / 1024) + " MB");
  } else {
    print("  Total Flash Size : Unable to read");
  }
  print("  Flash Placement  : " + (esp.embeddedFlash ? "Embedded in SoC Die" : "External Module"));
  print("\n[ JAVASCRIPT ENGINE ENVIRONMENT ]");
  print("  JS Vars Used     : " + mem.usage);
  print("  JS Vars Free     : " + mem.free + " (Available)");
  print("  Total Alloc Vars : " + mem.total);
  print("  History Tracked  : " + mem.history + " items");
  print("  GC Total Runs    : " + mem.gc + " cycles (Last: " + (mem.gctime ? mem.gctime.toFixed(1) + "ms" : "0ms") + ")");
  print("\n[ ACTIVE RADIO PERIPHERALS ]");
  print("  Wi-Fi Subsystem  : " + (esp.Wifi ? "ONLINE" : "OFFLINE"));
  print("  Bluetooth LE     : " + (esp.BLE ? "ONLINE" : "OFFLINE"));
};

printCompleteSpecs();