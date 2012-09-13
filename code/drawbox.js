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

for (i=0;i<4;i+=0.1) print(JSON.stringify(boxcoords(i)));

var pos = 0.0;
setInterval(function() {
 pos+=0.01;
 var coords = boxcoords(pos);
 //print(JSON.stringify(coords));
 pulse("A1",1,1+coords[0]);
 pulse("A2",1,1+coords[1]);
},100);

clearInterval(0);

var pos = 0.0;
var coords = [];
setInterval(function() {
 pos+=0.005;
 coords = boxcoords(pos);
 pulse("A1",1,1+coords[0]);
 pulse("A2",1,1+coords[1]);
},50);


