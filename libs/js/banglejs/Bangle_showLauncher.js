(function() {
  var launcherApp=(require("Storage").readJSON("setting.json",1)||{}).launcher;
  if (require("Storage").read(launcherApp)==undefined) {
    launcherApp = require("Storage").list(/\.info$/)
      .map(file => {
        const app = require("Storage").readJSON(file,1);
        if (app && app.type == "launch") {
          return app;
        }
      })
      .filter(x=>x)
      .sort((a, b) => a.sortorder - b.sortorder)[0].src;
  }

  if (require("Storage").read(launcherApp)==undefined) {
    eval(`E.showMessage("No Launcher Found");setWatch(()=>{load();}, global.BTN2||BTN, {repeat:false,edge:"falling"});`);
  } else {
    if (Bangle.uiRemove) {
      Bangle.setUI(); // remove all existing UI (and call Bangle.uiRemove)
      setTimeout(eval,0,require("Storage").read(launcherApp)); // Load launcher direct without a reboot
    } else load(launcherApp);
  }
  delete launcherApp;
})
