// issue with function decls when the last token is right at the end of the line

var a=eval("function a() {\naaaaaaaaa\nssssssss\n}")
var r=a.toString();

result = r=="function () {\naaaaaaaaa\nssssssss\n}";
