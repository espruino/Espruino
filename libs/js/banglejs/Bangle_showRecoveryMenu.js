(function() {
  clearTimeout();
  clearInterval();
  clearWatch();
  Bangle.removeAllListeners();
  E.removeAllListeners();
  NRF.removeAllListeners();
  Bangle.setLCDBrightness(1);
  E.showMenu({"":{title:"Recovery"},
    "Clean Boot": () => {
      reset();
    },  
    "Reboot": () => {
      E.reboot();
    },
    "Turn Off": () => {
      Bangle.off();
    },
    "Factory Reset": () => {
      E.showPrompt("Are you sure?\nThis will remove all data.",{title:"Factory Reset"}).then(ok => {
        if (!ok) return Bangle.showRecoveryMenu();
        E.showMessage("Resetting");
        Bangle.setLCDTimeout(0);
        if(!NRF.getSecurityStatus().connected)
          Terminal.setConsole();
        Bangle.factoryReset();
      });
    },    
    "Exit": () => {   
      if (require("Storage").list().length>0) {
        E.showMessage("Loading...");
        if(!NRF.getSecurityStatus().connected)
          Terminal.setConsole();
        load();
      } else {
        E.reboot();
      }
    },
    "Attempt Compact": () => {
      E.showMessage("Compacting...\nMay take\n5 min.");
      if(!NRF.getSecurityStatus().connected)
        Terminal.setConsole();
      require("Storage").compact();
      E.reboot();
    },
    "Rewrite Bootloader": () => {
      setTimeout(load,1000);
      eval(require("Storage").read("bootupdate.js"));
    },
  });
})
