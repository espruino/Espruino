// Garbage collection test

var a = {};

for (i=0;i<10;i++) { 
  a = {"12345678901234567890":"asdfghjkl;zxcvbnm,wertyuioiuytredscfvghjkmnbvfdrtyujknbvcfdrtyuikmnbgv"};
  // create loop
  a.b = { c : { d : a } };
  a = undefined;
}

result = 1;
