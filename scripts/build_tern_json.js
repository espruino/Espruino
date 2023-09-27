#!/usr/bin/env node

// sudo npm install -g marked highlight.js

// This build a JSON description file for Tern.js as
// specified here: http://ternjs.net/doc/manual.html#typedef

var marked = require('marked');
// yay - new marked version incompatible with old one...
if (marked.marked) marked=marked.marked; 
// Set default options except highlight which has no default
marked.setOptions({
  gfm: true, // github markdown
  highlight: function (code) {
    return require('highlight.js').highlightAuto(code).value;
  },
  tables: true,
  breaks: false,
  pedantic: false,
  sanitize: false,
  smartLists: true,
  smartypants: false,
  langPrefix: 'lang-'
});

var hadErrors = false;

require("./common.js").readAllWrapperFiles(function(json) {
  var tern = { "!name": "Espruino" };

  // Handle classes/libraries first
  json.forEach(function (j) {
    try {
      if (j.type=="class") {
        var o = { "!type": "fn()" };
        o["!doc"] = marked(j.getDescription());
        o["!url"] = j.getURL();
        tern[j.class] = o;

        if ("prototype" in j) {
          o["prototype"] = { "!proto": j.prototype+".prototype" };
        }
      } else if (j.type=="object") {
        var o = { "!type": ("instanceof" in j) ? ("+"+j.instanceof) : "?" };
        o["!doc"] = marked(j.getDescription());
        o["!url"] = j.getURL();
        tern[j.name] = o;
      } else if (j.type=="library") {
        // TODO: bind this into 'require' somehow
        var o = { "!type": "fn()" };
        o["!doc"] = marked(j.getDescription());
        o["!url"] = j.getURL();
        tern[j.class] = o;
      }
    } catch (e) {
      console.error("Exception "+e, j);
      hadErrors = true;
    }
  });

  // Handle contents
  json.forEach(function (j) {
    try {
      if (["include"].indexOf(j.type)>=0) {
        // meh
      } else if (["class","object","library"].indexOf(j.type)>=0) {
        // aready handled above
      } else if (["init","idle","kill","hwinit"].indexOf(j.type)>=0) {
        // internal
      } else if (["event"].indexOf(j.type)>=0) {
        // TODO: handle events
      } else if (["constructor"].indexOf(j.type)>=0) {
        var o = tern[j.class]; // overwrite existing class with constructor
        if (o===undefined) o=tern[j.class]={};
        o["!type"] = j.getTernType();
        o["!doc"] = marked(j.getDescription());
        o["!url"] = j.getURL();
      } else if (["staticproperty","staticmethod"].indexOf(j.type)>=0) {
        var o = { "!type": j.getTernType() };
        o["!doc"] = marked(j.getDescription());
        o["!url"] = j.getURL();
        tern[j.class][j.name] = o;
      } else if (["property","method"].indexOf(j.type)>=0) {
        var o = { "!type": j.getTernType() };
        o["!doc"] = marked(j.getDescription());
        o["!url"] = j.getURL();
        if (!("prototype" in tern[j.class])) {
          tern[j.class]["prototype"] = { };
          if (["Object","Function","Array","String","Number","Boolean","RegExp"].indexOf(j.class)>=0)
             tern[j.class]["prototype"]["!stdProto"] = j.class;
        }
        tern[j.class]["prototype"][j.name] = o;
      } else if (["function","variable"].indexOf(j.type)>=0) {      
        var o = { "!type": j.getTernType() };
        o["!doc"] = marked(j.getDescription());
        o["!url"] = j.getURL();
        tern[j.name] = o;
      } else if (j.type.startsWith("EV_SERIAL")) {
        // internal - no type info needed
      } else
       console.warn("Unknown type "+j.type+" for ",j);
    } catch (e) {
      console.error("Exception "+e, e.stack, j);
      hadErrors = true;
    }
  });

 // FIXME: not sure why (because Telnet has children when it shouldn't?) but this breaks tern's parsing :(
 delete tern["Telnet"]["!type"];


 if (hadErrors) {
   console.log("ERROR PROCESSING JSWRAP FILES");
   process.exit(1);
 }
 console.log(JSON.stringify(tern,null,2));

});
