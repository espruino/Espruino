z = 0;

function addstuff() {
 var count=0;
 z = function() { 
  count++;
  return count;  
 };
}

addstuff();


result = z();
