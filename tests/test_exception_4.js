a = [];
try {
  a.push("Before");
} catch (e) {
  a.push("Catch");
} finally {
  a.push("Finally");
}
a.push("After");

a = a.join(",");
result = a=="Before,Finally,After";
