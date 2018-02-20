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
  static isDog() {
    return false;
  }
}

class Lion extends Cat {
  speak() {
    return super.speak()+this.name + ' roars.';
  }
  static isReallyADog() {
    return super.isDog();
  }
}

class Lion2 extends Cat {
  constructor(name) {
    super(name);
  }
  speak() {
    return super.speak()+this.name + ' roars.';
  }
}

var c = new Cat("Tiddles");
var l = new Lion("Alan");
var l2 = new Lion("Nigel");
var rb = c.speak()=="Tiddles makes a noise." && l.speak()=="Alan makes a noise.Alan roars." && l2.speak()=="Nigel makes a noise.Nigel roars." && Lion.isReallyADog()===false;

// --------------------------------------------

result = ra && rb; 

