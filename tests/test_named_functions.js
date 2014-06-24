// Function expressions and named functions. #77

var test1 = function fibo(x) {
  if (x < 2) {
    return x;
  } else {
    return (fibo(x - 1) + fibo(x - 2));
  }
}

function tester(x) { 
  var r = test1(x);
  print(x,"=",r);
  return r;
}

result = tester(1)==1 && tester(2)==1 && tester(3)==2 && tester(4)==3 && tester(5)==5;;
