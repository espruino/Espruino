// https://github.com/espruino/Espruino/issues/488
// Espruino locks up if a known module name is used as an unknown module method or property

try {
  a.E
  a["E"]
} catch (e) {
  result=1;
}

