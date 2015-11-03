// issue with function decls when the last token is right at the end of the line

var a=eval("function a() {\n  aaaaaaaaa\nssssssss\n}")
var r=a.toString();

result = r=="function () {\n  aaaaaaaaa\nssssssss\n}";
