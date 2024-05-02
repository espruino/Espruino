// roughly based on https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Classes
// and https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/super
class Rectangle {
  constructor(height, width) {
    this.height = height;
    this.width = width;
  }

  // Method
  calcArea() {
    return this.height * this.width;
  }

  static isACircle() {
    return false;
  }
}

var p = new Rectangle(4,3);
var ra = p.width==3 && p.height==4 && p.calcArea()==12 && !Rectangle.isACircle();

// --------------------------------------------

class Cat { 
  constructor(name) {
    this.name = name;
  }  
  speak() {
    return this.name + ' makes a noise.';
  }  
  getLegs() {
    return 4;
  }  
  static isDog() {
    return false;
  }
  field = 42
  static staticField = 43
}

class Lion extends Cat {
  speak() {    
    return super.speak()+this.name + ' roars.';
  }
  static isReallyADog() {
    return super.isDog();
  }
}

class Lion2 extends Lion {
}

var c = new Cat("Tiddles");
var l = new Lion("Alan");
var l2 = new Lion("Nigel"); // NOTE: making this an instance of Lion2 breaks things
var rb = l.getLegs()==4 && c.speak()=="Tiddles makes a noise." && l.speak()=="Alan makes a noise.Alan roars." && l2.speak()=="Nigel makes a noise.Nigel roars." && Lion.isReallyADog()===false;
var rc = c.field==42;
var rd = Cat.staticField==43;

// --------------------------------------------

result = ra && rb && rc && rd; 

