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

console.log("Results:",results);

result = results.every(r=>r);
