// deleting the last element in array #346
// Deleting the last element shouldn't change array's length

var arr = ["a", "b", "c"];
delete arr[1];
a = arr.length; //returns 3 as expexted


var arr = ["a", "b", "c"];
delete arr[2];
b = arr.length; //returns 2, but 3 was expected


result = a==3 && b==3;
