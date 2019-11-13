// https://github.com/espruino/Espruino/issues/894#issuecomment-402553934


  var promise = new Promise(function (resolve) {
    setTimeout(resolve, 10);
  });

  promise.then(function () {
    console.log('first');

    promise.then(function () {
      console.log('second'); // wasn't called
      result = 1;
    });
  });

  setTimeout(function () {
    console.log('ready for fourth promise');

    promise.then(function () {
      console.log('third');
    });
  }, 15);


