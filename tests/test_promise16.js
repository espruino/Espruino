// https://github.com/espruino/Espruino/issues/2450 part 2
// test chaining with catch...
var sequence = "";

let p = new Promise((r)=>{
  sequence += "C";
  r(new Promise((r2)=>{
    setTimeout(()=>{
      sequence += "B";
      //r2();
    },2);
  }));
});

p.then((v)=>{
  sequence += "A";
});

setTimeout(function() {
  result = sequence == "CB";
  console.log(result, sequence);
},10);
