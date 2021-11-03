// https://github.com/espruino/Espruino/issues/2086
E.setFlags({pretokenise:1})
a = () => print(1 - -1)
a();
result=1;
