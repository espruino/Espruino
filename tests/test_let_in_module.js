// http://forum.espruino.com/conversations/376541/
// https://github.com/espruino/Espruino/issues/2215
// https://github.com/espruino/Espruino/issues/2207

()=>{
  const x = 5;
  
  Modules.addCached("issue2215", function() {
    function test(p){
      console.log("Test", p);
    }

    let variable_let = { test: test };
    const variable_const = { test: test };
    var variable_var = { test: test };

    exports.func_let = function (){
      variable_let.test("let");
    };

    exports.func_const = function (){
      variable_const.test("const");
    };

    exports.func_var = function (){
      variable_var.test("var");
    };
  });

  var test = require("issue2215");
  test.func_var(); // ok
  test.func_let(); // fail
  test.func_const(); // fail
  result = true;
}();

