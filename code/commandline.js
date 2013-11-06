var cmd="";
Serial1.onData(function (e) {
  if (e.data=="\r") {
    print("'"+cmd+"' = "+eval(cmd));
    cmd="";
  } else cmd+=e.data;
});

var cmd="";
Serial1.onData(function (e) {
  Serial1.print(e.data);
  if (e.data=="\r") {
    var s = "'"+cmd+"' = "+eval(cmd);
    print(s);
    Serial1.println(s);
    cmd="";
  } else cmd+=e.data;
});
