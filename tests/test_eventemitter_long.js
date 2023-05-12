//https://github.com/espruino/Espruino/issues/906

var obj = {};
obj.on('areallyreallyreallylongeventname', function () { console.log("Fired!"); result=1; });
trace(obj);
obj.emit('areallyreallyreallylongeventname');
