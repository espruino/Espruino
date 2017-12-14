var alreadyCompleted1 = new Promise( function(resolve,reject) { resolve(42); } );
var alreadyCompleted2 = new Promise( function(resolve,reject) { resolve(); } );

setTimeout(function() {
  var p1 = Promise.resolve(3);
  var p2 = 1337;
  var p3 = new Promise((resolve, reject) => {
    setTimeout(resolve, 100, "foo");
  }); 

  Promise.all([alreadyCompleted1,alreadyCompleted2,p1, p2, p3]).then(values => { 
    allresult = values.toString();
    result = allresult=="42,,3,1337,foo";
  });
}, 1);
