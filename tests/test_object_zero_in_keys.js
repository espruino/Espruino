// https://github.com/espruino/Espruino/issues/2176

a = { "" : {}, " \x00hello" : "world" }
a["\x00abc"] = "foo"
a[" \x00abc"] = "bar"

json = JSON.stringify(a)
result = json == "{\"\":{},\" \\u0000hello\":\"world\",\"\\u0000abc\":\"foo\",\" \\u0000abc\":\"bar\"}";
