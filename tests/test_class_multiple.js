// Issue 1529 - part 1

class A {
    constructor(n) {
        this.name = n;
    }
}
class B extends A {
}
class C extends B {
}
new A(); // ok
new B(); // ok
var cInst = new C("Test"); // was stack overflow

// Issue 1529 - part 2
class Being  { 
    constructor(theLabel) {
        this.label = theLabel;
    }

    name() {
        return this.label;
    }
}

class Animal extends Being { 
    constructor(theLabel) {
        super(theLabel);
    }
}

class Dog extends Animal { 
    constructor(theLabel) {
        super(theLabel);
    }
}

class Cat extends Animal {
    constructor(theLabel) {
        super(theLabel);
    }
 }


const c = new Cat("Felix");
const d = new Dog("Fido");
result = cInst.name=="Test" && c.name()=="Felix" && d.name()=="Fido";
