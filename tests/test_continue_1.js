// https://github.com/espruino/Espruino/issues/501

// nested continue;

var r = 0;

for (var i=0; i < 10; ++i) {
  for (var j=0; j < 10; ++j);
  if(i==0) continue;
  r++;
}

for (var i=0; i < 10; ++i) {
  for (var j=0; j < 10; ++j);
  if(i==1) continue;
  r++;
}


result = r==18;
