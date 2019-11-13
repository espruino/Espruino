// https://github.com/espruino/Espruino/issues/1476

var promise = new Promise(function (resolve) {
    setTimeout(resolve, 10);
  });

  promise.then(function () {
    console.log('first');

    promise.then(function () {
      console.log('third');
    });
  });

  promise.then(function () {
    console.log('second');
  });

  setTimeout(function () {
    console.log('ready for fourth promise');

    promise.then(function () {
      console.log('fourth');
      result = 1;
    });
  }, 15);
