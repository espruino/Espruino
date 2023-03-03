// https://github.com/espruino/Espruino/issues/2338#issuecomment-1453477606

var success = [];

// Not reaching finally block
(() => {
  try {
    console.log("testing return...");
    return;
  } finally {
    console.log("return success");
    success.push("return");
  }
})();

(() => {
  do {
    try {
      console.log("testing break...");
      break;
    } finally {
      console.log("break success");
      success.push("break");
    }
  } while (false);
})();

(() => {
  do {
    try {
      console.log("testing continue...");
      continue;
    } finally {
      console.log("continue success");
      success.push("continue");
    }
  } while (false);
})();

// we shouldn't execute finally in these cases!
(() => {
  return;
  try {
    print("uh-oh!");
  } finally {
    console.log("return fail");
    success.push("return-FAIL");
  }
})();
try {
  (() => {
    throw new Error("Boom!");
    try {
      print("uh-oh!");
    } finally {
      console.log("return fail");
      success.push("return-FAIL");
    }
  })();
} catch (e) {}

result = success=="return,break,continue";
