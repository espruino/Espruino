// see:
//  libs/misc/jswrap_curio.c
//  boards/CURIO.py
led([0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0]);

const PID = {
  // gains
  Pg : 0.015,
  Ig : 0.001,
  Dg : 0.05,
  // min/max
  Imin : -0.5,
  Imax : 0.5
};
const CONST = {
  MAXSPEED : 25, // steps/sec
  MINSPEED : 15, // steps/sec - slowest we can sensibly run
  SLOW_STEPS : 10, // how many steps before we slow down
};
function Motor(M1,M2) {
  Object.assign(this,{
    M1:M1,M2:M2,
    aspeed : 0, // actual speed
    rspeed : 0,   // requested speed
    duty : 0,
    idle : 0,
    lastTime : getTime(), // time of last pin event
    pwm : undefined, // PWM pin
    direction : 0, // 1 or -1
    odometer : 0, // how many steps have we moved?
    // move to location
    stepsLeft : 0,
    stepSpeed : 0, // if set, we do a certain number of steps
    // PID
    err:0,
    P:0,I:0,D:0,
  });
}
Motor.prototype.setSpeed = function(speed) {
  if (speed==0) {
    digitalWrite([this.M1,this.M2],0);
    this.duty = 0;
    this.rspeed = 0;
    this.pwm = undefined;
    this.P = this.I = this.D = this.err = 0;
  } else {
    if (speed>0) {
      this.pwm = this.M1;
      this.M2.reset();
      this.direction = 1;
    } else  if (speed<0) {
      this.pwm = this.M2;
      this.M1.reset();
      this.direction = -1;
    }
    this.duty = 1;
    this.idle = 0;
    analogWrite(this.pwm,this.duty);
  }
  this.rspeed = Math.abs(speed);
};
Motor.prototype.moveSteps = function(steps, speed) {
  if (!speed || speed<0) speed = CONST.MAXSPEED;
  if (steps<1) {
    this.setSpeed(-speed);
  } else {
    this.setSpeed(speed);
  }
  this.stepSpeed = speed;
  this.stepsLeft = Math.abs(steps);
};
Motor.prototype.onSensor = function(e) {"ram";
  var d = e.time-this.lastTime;
  if (d<0.02) return;
  //print(d.toFixed(3));
  this.lastTime = e.time;
  this.odometer += this.direction;
  if (this.stepsLeft) this.stepsLeft--;
  this.aspeed = this.aspeed*0.6 + (0.4/d);
  //print(this.aspeed, this.lastTime, this.idle);
  this.idle=0;
};
Motor.prototype.onStep = function() {"ram";
  this.idle++;
  if (this.idle>10)
    this.aspeed = this.aspeed*0.6 + (0.4 / (getTime()-this.lastTime));
  // Stepping
  if (this.stepSpeed) {
    if (this.stepsLeft >= CONST.SLOW_STEPS) {
      this.rspeed = this.stepSpeed;
    } if (this.stepsLeft) {
      this.rspeed = ((this.stepSpeed * this.stepsLeft / CONST.SLOW_STEPS) + CONST.MINSPEED) / 2;
    } else {
      this.stepSpeed = 0;
      this.setSpeed(0);
      // complete!
    }
  }
  // PID
  var err = this.rspeed-this.aspeed;
  this.P = this.err * PID.Pg;
  this.D = (err-this.err) * PID.Dg;
  this.err = err;
  this.I = E.clip(this.I + err * PID.Ig, PID.Imin, PID.Imax);
  this.duty = E.clip(0.5 + this.P + this.I + this.D, 0, 1);
  if (this.pwm)
    analogWrite(this.pwm, this.duty);
};

L = new Motor(ML1,ML2);
R = new Motor(MR1,MR2);
setInterval(function() {"ram";
  L.onStep();
  R.onStep();
}, 20);
clearWatch();
setWatch(L.onSensor.bind(L), IRL, {repeat:true, edge:0});
setWatch(R.onSensor.bind(R), IRR, {repeat:true, edge:0});

function go(l,r) {
  L.setSpeed(l);
  R.setSpeed(r);
}
function move(l,r,lspeed,rspeed) {
  L.moveSteps(l,lspeed);
  R.moveSteps(r,rspeed);
}

