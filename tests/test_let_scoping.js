var results=[];

{
  let x = 1;
  if (x === 1) {
    let x = 2;
    console.log(x); results.push(x==2);
  }
  console.log(x); results.push(x==1);
}

function test1() {
  let x = 1;
  if (x === 1) {
    let x = 2;
    console.log(x); results.push(x==2);
  }
  console.log(x); results.push(x==1);
}

test1();


function test2() {
  var x = 1;
 
  if (x === 1) {
    let x = 2;
    var y = 3;
    let z = 4;
    
    console.log(x); results.push(x==2);
    console.log(y); results.push(y==3);

  }

  console.log(x); results.push(x==1);
  console.log(y); results.push(y==3);
  console.log(typeof z);results.push("undefined" == typeof z);
}

test2();


function test3() {
  for(let g=0;g<1;g++);
  results.push("undefined" == typeof g);
}
test3();


// https://github.com/espruino/Espruino/issues/2224
function pushCommand(command) {
  let hash = print(arguments); // fails
  var hash2 = print(arguments); // works
  print(arguments); // works
  results.push(true);
}
pushCommand("Hello")

console.log("Results:",results);

result = results.every(r=>r);
