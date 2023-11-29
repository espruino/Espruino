try {
  throw "Oh Noes!";
} finally {
  console.log("Finally block executed");
  result = 1;
}
console.log("exception uncaught so nothing from here is executed");
result = 0;
try {
  result = 0;
} catch (e) {
  result = 0;
} finally {
  result = 0;
}
