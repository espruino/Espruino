(function() {
  Bangle.setUI();
  clearTimeout();
  clearInterval();
  clearWatch();
  Bangle.removeAllListeners();
  E.removeAllListeners();
  NRF.removeAllListeners();
  Bangle.setLCDBrightness(1);
  let menu = {"":{title:"Recovery"},
    "Clean Boot": () => {
      reset();
    },
    "Reboot": () => {
      E.reboot();
    },
    "Turn Off": () => {
      Bangle.off();
    },
    "Delete BT Pairing": () => {
      E.showPrompt("Delete BT Pairing Info?",{title:"Bluetooth", buttons : {"No":0,"Soft":1,"Hard":2}}).then(type => {
        if (!type) return Bangle.showRecoveryMenu();
        NRF.disconnect();
        Terminal.setConsole();
        setTimeout(function() {
          NRF.eraseBonds( type == 2/*hard*/ );
          E.showMessage("Done!");
        }, 500);
        setTimeout(E.reboot, 1000);
      });
    }
  };
  if (process.env.BOARD=="BANGLEJS2" || process.env.BOARD=="BANGLEJS3")
    Object.assign(menu, {"Test": Bangle.showTestScreen});
  Object.assign(menu, {"Factory Reset": () => {
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
  E.showMenu(menu);
})
