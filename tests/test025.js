// Array length test

myArray = [ 1, 2, 3, 4, 5 ];
myArray2 = [ 1, 2, 3, 4, 5 ];
myArray2[8] = 42;

result = myArray.length == 5 && myArray2.length == 9;
