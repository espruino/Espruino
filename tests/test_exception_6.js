function b() {  
  throw new Error("Some Error");
  return [1];
};

function a() {
  return b().length>0;
}

try {
  a();
} catch (e) {
  console.log("--->"+e.toString());
  result = e.toString() == "Error: Some Error";
}
console.log(result);
