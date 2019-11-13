// timer again

var foo = setTimeout("result=1",50);

function clearAlreadyTriggeredTimeout() {
  try {
    clearTimeout(foo);
  } catch(e) {
    // should not throw when the `foo` timeout was already executed
    result=0
  }
}

setTimeout(clearAlreadyTriggeredTimeout, 100);
