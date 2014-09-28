// https://github.com/espruino/Espruino/issues/428

function broken() {
  switch(1) {
    case 1: working();
    break;
  }
 }
 
 function working() {  
    for(var i=0;i<10;i++){ 
      if (i>5) {
        console.log("Hello");
        break;
      }
    }
   console.log("World");
   result = 1;
 }


broken();
