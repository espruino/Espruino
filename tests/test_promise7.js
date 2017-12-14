//http://forum.espruino.com/conversations/297505/#comment13375674
new Promise((x,y)=>x()).then(dev=>{
  return new Promise((x,y)=>y("Uh-oh"));
}).then(s=>{
  console.log("ok", s);
}).catch(e=>{
  console.log("expected error", e);
  result = e == "Uh-oh";
});
