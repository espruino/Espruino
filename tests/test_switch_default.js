// https://github.com/espruino/Espruino/issues/964
switch (5) {
  case 5:
  default:
    console.log( "I should be called!" );
    result = 1;
}
