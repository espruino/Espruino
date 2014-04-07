// test for array join
var a = [1,2,4,5,7];

// https://github.com/espruino/Espruino/issues/289
var b = ["true", undefined, true, null];

result = a.join(",")=="1,2,4,5,7" && b.join()=="true,,true,";
