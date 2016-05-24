// https://github.com/espruino/Espruino/issues/845

function test(){
 switch ("a"){
  case "a":
   eval(1);
   break;
 }
}

try {
  test();
  result = 1;
} catch (e) {
  result = 0;
}
