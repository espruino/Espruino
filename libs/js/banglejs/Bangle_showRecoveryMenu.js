(function() {
  clearTimeout();
  clearInterval();
  clearWatch();
  Bangle.removeAllListeners();
  E.removeAllListeners();
  NRF.removeAllListeners();
  E.showMenu({"":{title:"Recovery"},
    "Attempt Compact": () => {
      E.showMessage("Compacting...\nMay take\n5 min.");
      if(!NRF.getSecurityStatus().connected)
        Terminal.setConsole();
      require("Storage").compact();
      E.reboot();
    },
    "Rewrite bootloader": () => {
      setTimeout(load,1000);
      eval(require("Storage").read("bootupdate.js"));
    },
    "Factory Reset": () => {
      E.showPrompt("Are you sure?\nThis will remove all data.",{title:"Factory Reset"}).then(ok => {
        if (!ok) return Bangle.showRecoveryMenu();
        E.showMessage("Resetting");
        if(!NRF.getSecurityStatus().connected)
          Terminal.setConsole();
        Bangle.factoryReset();
      });
    },
    "Clean Boot": () => {
      reset();
    },
    "Reboot": () => {
      E.reboot();
    },
    "Turn Off": () => {
      Bangle.off();
    },
    "Exit": () => {
      E.showMessage("Loading...");
      if(!NRF.getSecurityStatus().connected)
        Terminal.setConsole();
      load();
    },
  });
})