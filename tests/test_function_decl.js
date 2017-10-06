// issue with function decls when the last token is right at the end of the line

var a=eval("function a() {\n  12345\nssssssss\n}")
var r=a.toString();

result = r.replace(/\s/g,"")=="function(){12345ssssssss}";
