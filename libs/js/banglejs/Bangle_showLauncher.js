(function() {
  let s = require("Storage").readJSON("setting.json",1)||{};
  let launcherApp = require("Storage").read(s.launcher);

  if (!launcherApp) {
    launcherApp = undefined; // configured app not found
    let info = require("Storage").list(/\.info$/)
      .map(file => {
        const app = require("Storage").readJSON(file,1);
        if (app && app.type == "launch") {
          return app;
        }
      })
      .filter(x=>x)
      .sort((a, b) => a.sortorder - b.sortorder)[0];
    if (info) {
      s.launcher = info.src;
      require("Storage").writeJSON("setting.json",s);
      launcherApp = require("Storage").read(info.src);
    }
  }

  if (!launcherApp) {
    eval(`E.showMessage("No Launcher Found");setWatch(()=>{load();}, global.BTN2||BTN, {repeat:false,edge:"falling"});`);
  } else {
    if (Bangle.uiRemove) {
      Bangle.setUI(); // remove all existing UI (and call Bangle.uiRemove)
      setTimeout(eval,0,code); // Load launcher direct without a reboot
    } else load(s.launcher);
  }
  delete s;
  delete launcherApp;
})
