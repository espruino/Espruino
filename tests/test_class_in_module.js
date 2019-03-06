/*

Good:


#39[r3,l1] Function { 
  #40[r1,l2] Name Param "\/a"     undefined 
  #45[r1,l2] Name String [1 blocks] "\/cod"    #41[r1,l0] FlatString [4 blocks] "console.log(\"Argument\",a);\n  if (a!=\"Foo\") throw \"No argument passed!\";"  
  #46[r1,l2] Name String [1 blocks] "\/sco"    #30[r2,l0] Object { 
      #28[r1,l2] Name String [1 blocks] "exports"        #39[r3,l1] ...
 
      #37[r1,l2] Name String [1 blocks] "module"        #30[r2,l0] ...
 
      #38[r1,l2] Name String [1 blocks] "b"        #39[r3,l1] ...
 
    } 
}
Argument Foo

Bad:

#41[r4,l1] Function { 
  #42[r1,l2] Name String [1 blocks] "\/sco"    #32[r2,l0] Object { 
      #30[r1,l2] Name String [1 blocks] "exports"        #41[r4,l1] ...
 
      #29[r1,l2] Name String [1 blocks] "module"        #32[r2,l0] ...
 
      #40[r1,l2] Name String [1 blocks] "b"        #41[r4,l1] ...
 
    } 
  #57[r1,l2] Name Param "\/a"     undefined 
  #58[r1,l2] Name String [1 blocks] "\/cod"    #59[r1,l0] String [4 blocks] "console.log(\"Argument\",a);\n    if (a!=\"Foo\") throw \"No argument passed!\";"  
  #43[r1,l2] Name String [2 blocks] "prototype"    #45[r1,l0] Object { 
      #46[r1,l2] Name String [2 blocks] "constructor"        #41[r4,l1] ...
 
    } 
}

- scope is too soon
*/
Modules.addCached("mod",`
class b {
  constructor(a) {
    console.log("Argument",a);
    if (a!="Foo") throw "No argument passed!";
    this.foo=a;
  }
}
exports = b;`);

//trace(require('mod'));
m = new (require('mod'))("Foo");
result = m.foo == "Foo";
