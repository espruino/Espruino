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

try { // Optional catch Binding
  a.push("Before");
  throw "Oh Noes!";
  a.push("After");
} catch {
  a.push("Catch");
} finally {
  a.push("Finally");
}
a.push("After");

a = a.join(",");
result = a=="Before,Catch,Finally,After,Before,Catch,Finally,After";
