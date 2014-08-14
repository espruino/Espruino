// simple test that standard minified version of stepper driver doesn't cause errors
function stepper(pins,frequency,step4,forward){var step,steps,duration,interval,direction;step=0;if(step4)steps=[1,2,4,8];else steps=[1,3,2,6,4,12,8,9];duration=1E3/frequency;direction=forward;function doStep(){if(direction){step++;step=step%steps.length}else{step--;if(step<0)step=steps.length-1}digitalWrite(pins,steps[step]);result=1;}this.start=function(){interval=setInterval(function(){doStep()},duration)};this.stop=function(){clearInterval(interval)};this.changeDirection=function(forward){this.stop();direction=
forward;this.start()};this.start()}var st=new stepper([D0,D1,D2,D3],100,true,true);
setTimeout("st.stop();",10);
