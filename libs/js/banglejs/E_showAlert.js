(function(msg,options) {
  if ("string" == typeof options)
    options = { title : options };
  options = options||{};
  options.buttons = {Ok:1};
  options.img  = require("heatshrink").decompress(atob("lEo4UBov+///BIMggFVAAQHBAoYIEBQ1QBIcFBIdABIcBBAVUHYsVDgweEDggeELQ4JKGAcP+AyDGAcO2AyDBJI6DBIkBBLpKDBIgAEBOKBEABSyGMYwJTGIkBWabRJd6dVIw0BBIIyDGAYJBGQYwEDwwcCDwwcCAAQ5FBQwFDA="));
  return E.showPrompt(msg,options);
})
