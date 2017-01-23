var tests=0, pass=0;

function t(expr, res) {
  tests++;
  var r = eval(expr);
  if (r.toString()==res.toString())
    pass++;
  else
    console.log(expr+" should equal "+res+" but got "+r);
}

// bool conversion
t("!!new Uint8Array([])",true)
t("!!new Uint8Array([0])",true)
t("!!new Uint8Array([1])",true)
t("!!new Uint8Array([1,2])",true)
// int/float conversion
t("new Uint8Array([1,2])-0",NaN)
t("0|new Uint8Array([])",0)
t("0|new Uint8Array([0])",0)
t("0|new Uint8Array([42])",42)
t("0|new Uint16Array([42])",42)
t("0|new Int16Array([42])",42)
t("new Float32Array([42])-0",42)
t("new Float64Array([1.2])-0",1.2)

result = tests==pass;
