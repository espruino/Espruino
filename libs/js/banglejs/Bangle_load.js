(function(name) {
  if (Bangle.uiRemove) {
    Bangle.setUI(); // remove all existing UI (and call Bangle.uiRemove)
    __FILE__=name;
    if (!name) name = ".bootcde";
    setTimeout(eval,0,require("Storage").read(name)); // Load app without a reboot
  } else load(name);
})
