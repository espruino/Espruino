try {
  throw "Oh Noes!";
} finally {
  result = 1;
}
result = 0;
try {
  result = 0;
} catch (e) {
  result = 0;
} finally {
  result = 0;
}
