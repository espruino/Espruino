(function() {
  let s = require("Storage").readJSON("setting.json",1)||{};
  let launcherApp = s.launcher;

  if (!launcherApp) {
    launcherApp = require("Storage").list(/\.info$/)
      .map(file => {
        const app = require("Storage").readJSON(file,1);
        if (app && app.type == "launch") {
          return app;
        }
      })
      .filter(x=>x)
      .sort((a, b) => a.sortorder - b.sortorder)[0].src;
    print(launcherApp);
    if (launcherApp) {
      s.launcher = launcherApp;
      require("Storage").writeJSON("setting.json",s);
    }
  }

  if (!launcherApp) {
    eval(`E.showMessage("No Launcher Found");setWatch(()=>{load();}, global.BTN2||BTN, {repeat:false,edge:"falling"});`);
  } else {
    if (Bangle.uiRemove) {
      Bangle.setUI(); // remove all existing UI (and call Bangle.uiRemove)
      setTimeout(eval,0,require("Storage").read(launcherApp)); // Load launcher direct without a reboot
    } else load(launcherApp);
  }
  delete s;
  delete launcherApp;
})
