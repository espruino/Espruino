result=1;
try {
  throw "Foo";
  while (true); // doesn't halt, but does reset
  print("hi");
  result=0;
} catch(err) {
 print("ERR:"+err);
}
