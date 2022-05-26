
var results = [];

const x = 42;
try {
  x = 43;
  results.push(false);
} catch (e) {
  results.push(true);
}

const y = 42;
try {
  y += 1;
  results.push(false);
} catch (e) {
  results.push(true);
}

// Normal FOR loops fail
try {
  for (const z=0;z<5;z++);
  results.push(false);
} catch (e) {
  results.push(true);
}

// FOR..IN loops are ok
for (const w in [1,2,3]) ;

print(results);
result = results.every(x=>x);
