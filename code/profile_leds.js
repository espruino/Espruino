var pos = 0.5;

function getPattern1() {
  var cols = [];
  for (var i=0;i<50;i++) {
     cols.push((1 + Math.sin((i+pos)*0.1324)) * 127);
     cols.push((1 + Math.sin((i+pos)*0.1654)) * 127);
     cols.push((1 + Math.sin((i+pos)*0.1)) * 127);
  }
  return cols;
}


function getPattern2() {
  var cols = "";
  for (var i=0;i<50;i++) {
     cols += String.fromCharCode((1 + Math.sin((i+pos)*0.1324)) * 127) +
             String.fromCharCode((1 + Math.sin((i+pos)*0.1654)) * 127) +
             String.fromCharCode((1 + Math.sin((i+pos)*0.1)) * 127);
  }
  return cols;
}

function getPattern3() {
  var cols = new Uint8Array(50*3);
  for (var i=0;i<50;i++) {
     cols[i*3]   = (1 + Math.sin((i+pos)*0.1324)) * 127;
     cols[i*3+1] = (1 + Math.sin((i+pos)*0.1654)) * 127;
     cols[i*3+2] = (1 + Math.sin((i+pos)*0.1)) * 127;
  }
  return cols;
}

function getPattern4() {
  var cols = new Uint8Array(50*3);
  var n = 0;
  for (var i=0;i<50;i++) {
     cols[n++] = (1 + Math.sin((i+pos)*0.1324)) * 127;
     cols[n++] = (1 + Math.sin((i+pos)*0.1654)) * 127;
     cols[n++] = (1 + Math.sin((i+pos)*0.1)) * 127;
  }
  return cols;
}


var t = [];
t.push(getTime());
for (var z=0;z<100;z++) getPattern1();
t.push(getTime());
for (var z=0;z<100;z++) getPattern2();
t.push(getTime());
for (var z=0;z<100;z++) getPattern3();
t.push(getTime());
for (var z=0;z<100;z++) getPattern4();
t.push(getTime());

for (i=0;i<t.length-1;i++)
  print("Time "+i+" = "+(t[i+1]-t[i]));

result = true;
