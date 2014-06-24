a = [];
try {
  a.push("Before");
  throw "Oh Noes!";
  a.push("After");
} catch (e) {
  a.push("Catch");
} finally {
  a.push("Finally");
}
a.push("After");

a = a.join(",");
result = a=="Before,Catch,Finally,After";
