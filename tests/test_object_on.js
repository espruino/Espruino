// Test for large event names. Recent bug found in .on
a = {};
a.on("abcdefghijklmnopqrstuvwxyz", "Hello") 
result = a["#onabcdefghijklmnopqrstuvwxyz"] == "Hello";
