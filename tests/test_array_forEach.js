//https://github.com/espruino/Espruino/issues/1962
// Array.forEach() does not loop through entire array if an entry is deleted

var foo = [1,2,3,4,5,6,7,8,9,10],
	bar = [3,7];
              
foo.forEach((entry, idx, arr) => {
	console.log("forEach: ", entry);

	if (bar.indexOf(entry) > -1 ) {
		arr.splice(idx, 1);
        	//arr[idx] = 0;    <--- Modifying a value instead of removing it works ok
	}
});

console.log("output: ", foo.toString());
result = foo.toString()=="1,2,4,5,6,8,9,10";
