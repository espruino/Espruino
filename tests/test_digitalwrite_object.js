var r = "";
var v = 0;
var pin = {
  something : "foo",
  read : function() { r+="r"+this.something; v=!v; return v; },
  write : function(v) { r+="w"+this.something+v },
};

r += digitalRead(pin);
r += digitalRead(pin);
digitalWrite(pin,1);
digitalWrite(pin,0);

result = r=="rfoo1rfoo0wfoo1wfoo0";
