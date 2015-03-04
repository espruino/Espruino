// https://github.com/espruino/Espruino/issues/501

var r=0;

for (var i=0; i < 10; ++i) {
  if(i==0) continue;
  for (var j=0; j < 0; ++j);
  r++;
}

for (var i=0; i < 10; ++i) {
  if(i==1) continue;
  for (var j=0; j < 0; ++j);
  r++;
}

result = r==18;
