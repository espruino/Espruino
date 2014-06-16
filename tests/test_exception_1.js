try {
  result = 1;
  throw "Oh Noes!";
  result = 0;
} finally {
  console.log("Finally executed");
}
