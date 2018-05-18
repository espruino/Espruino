// https://github.com/espruino/Espruino/issues/1352

var success = 0;

function test( crit){
  try{
    switch (crit) {
      case 1:  
        throw new Error('simulate crash 1');
        break;
      default:  
        throw new Error('simulate crash 2');
        break;
    }       
  }
  catch(err){
    console.log('CAUGHT',err);
    success++;
  }
}
try {
  test(1);
  test(2);
} catch (err) {
  success = 0;  
}
result = success==2;

