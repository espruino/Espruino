$fn=40;

module roundcube(W,H,R) {
 color("white") union() {
  translate([-W/2,-W/2,0]) cylinder(h=H,r=R,center=true);
  translate([W/2,-W/2,0]) cylinder(h=H,r=R,center=true);
  translate([W/2,W/2,0]) cylinder(h=H,r=R,center=true);
  translate([-W/2,W/2,0]) cylinder(h=H,r=R,center=true);
  cube([W+R*2,W,H],center=true);
  cube([W,W+R*2,H],center=true);
 }
}

module feet(X,H) {
 S=[2,1,1];
 XX=X/2+1;
 HH=-H/2;
 color("#303030") union() {
  translate([XX,-3,HH]) cube(S,center=true);
  translate([XX,-1.5,HH]) cube(S,center=true);
  translate([XX,0,HH]) cube(S,center=true);
  translate([XX,1.5,HH]) cube(S,center=true);
  translate([XX,3,HH]) cube(S,center=true);
 }
}

difference() {
 union() {
  roundcube(8,8,1);
  rotate([0,0,0]) feet(8,8);
  rotate([0,0,90]) feet(8,8);
  rotate([0,0,180]) feet(8,8);
  rotate([0,0,270]) feet(8,8);
  difference() {
   translate([0,6,0]) rotate([0,90,0]) roundcube(3,1,1);
   translate([0,6,0]) rotate([0,90,0]) roundcube(1,2,1);
  }
 }
 translate([0,0,1]) roundcube(6,8,1);
}
color("brown") roundcube(6,2,1);