function boxcoords(i) {
  while (i>4) i-=4;
  if (i<1) {
    return [i,0];
  } else if (i<2) {
    return [1,i-1];
  } else if (i<3) {
    return [3-i,1];
  } else {
    return [0,4-i];
  }
}

clearInterval(0);
var pos = 0.0;
var coords = [];
setInterval(function() {
 pos+=0.01;
 coords = boxcoords(pos);
 digitalPulse("A1",1,1+coords[0]);
 digitalPulse("A2",1,1+coords[1]);
},20);

setWatch(function() {
  if (digitalRead("A0")) {
    digitalWrite("C9",1);
    setTimeout(function() {
      digitalWrite("C9", 0);
      setTimeout(function() {
        digitalWrite("C9", 1);
        setTimeout(function() {
          digitalWrite("C9", 0);
        }, 301);
      }, 302);
    }, 303);
}}, "A0", true);

