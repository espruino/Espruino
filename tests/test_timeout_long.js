// https://github.com/espruino/Espruino/issues/416

result=1
setTimeout("result=0",24*60*60*1000);
setTimeout("clearTimeout()",50);
