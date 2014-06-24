var sumi = 0;
var sumj = 0;
var i,j;

for(i=0,j=7;i<10;i++,j++) { 
//  console.log(i,j);
  sumi+=i;
  sumj+=j;
}

result = sumi==45 && sumj==115;
