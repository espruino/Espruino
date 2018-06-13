// https://github.com/espruino/Espruino/issues/1433

new Promise(function(resolve, reject) {
  setTimeout(resolve, 100);
  setTimeout(reject, 200);
}).then(function() {
  console.log('resolved');
  result = 1;
}).catch(function() {
  console.log('rejected');
  result = 0; // shouldn't reject after initial resolve
});
