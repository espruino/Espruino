// https://github.com/espruino/Espruino/issues/1422

function meh() {                
  try {
    throw "Oh no";
  } catch(err) {
    return -1;
  }
}

result = meh() == -1;


