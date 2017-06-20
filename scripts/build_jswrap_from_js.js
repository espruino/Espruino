#!/usr/bin/node
/* This'll take JavaScript source files and convert them
to jswrap files to be included in the interpreter.

Anything that starts `exports.xyz =` will be added into
a library with the given name. The symbol table is hard-coded,
and each function is parsed every time it is used (although the
code for it is a native string, so will never take up space in RAM).

It's not super fast, but should be pretty efficient (especially when
used on minified files). The issue with just creating a function on
demand is the need to also add named variables for each parameter given.

While it could all be encoded, it's so much more simple to just execute
the complete function as-is.

*/

var acorn = require("acorn");

if (process.argv.length!=5) {
  console.log("USAGE: ");
  console.log("   build_jswrap_from_js.js LibraryName input.js jswrap_output.c");
} 
var LIBRARY_NAME = process.argv[2];
var FILE_IN = process.argv[3];
var FILE_OUT = process.argv[4];

function wrapFile(library, fn) {
  var cfile = 
`/* -----------------------------------------------
      ***** DO NOT MODIFY *****
      
    This file auto-generated from ${fn}  
    with scripts/build_jswrap_from_js.js
  ----------------------------------------------- */
    
`;
  
  var json = {
    type:"library",
    class:library
  }
  cfile += '/*JSON'+JSON.stringify(json,null,2)+'\n*/\n\n';
  var code = require("fs").readFileSync(fn).toString();
  try {
    var ast = acorn.parse(code, { ecmaVersion : 6 });
    //console.log(JSON.stringify(ast,null,2));
    var nodes = [];
    ast.body.forEach(function(node) {
      if (node.type=="ExpressionStatement" &&
          node.expression.type=="AssignmentExpression" &&
          node.expression.left.type=="MemberExpression" &&
          node.expression.left.object.type=="Identifier" &&
          node.expression.left.property.type=="Identifier" &&
          node.expression.left.object.name=="exports"/* &&
          node.expression.right.type=="FunctionExpression"*/) {
        var name = node.expression.left.property.name;
        var body = node.expression.right;
        var js = code.substring(body.start, body.end);
        if (node.expression.right.type=="FunctionExpression")
          js = '('+js+')';
        var json = {
          type : "staticproperty",
          class : library,
          "name" : name,
          // evaluate and treat string as static
          generate_full : "jspEvaluate("+JSON.stringify(js)+",true)",
          "return" : ["JsVar",""]
        };
        cfile += '/*JSON'+JSON.stringify(json,null,2)+'\n*/\n\n';
      } else {
        console.log("WARNING: un-handled statement tyoe "+JSON.stringify(code.substring(node.start, node.end)));
      }      
    });    
  } catch (err) {
    console.error(err);
  }
  return cfile;
}

console.log(`Converting ${FILE_IN} to ${FILE_OUT}`);
var out = wrapFile(LIBRARY_NAME, FILE_IN);
//if (out!==undefined)
  require("fs").writeFileSync(FILE_OUT, out);
