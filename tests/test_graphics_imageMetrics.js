var g = Graphics.createArrayBuffer(8,8,8);

var ok = true;
function SHOULD_BE(b,a) {
  if (a!=b) {
    console.log("GOT :"+b+"\nSHOULD BE:"+a+"\n================");
    ok = false;
  }
}


var m = g.imageMetrics(Graphics.createImage(`
           ########
          ##########
          ## #######
          ##########
          ##########
          ##########
          #####
          ########
#        #####
#      #######
##    ##########
###  ######### #
##############
##############
 ############
  ###########
   #########
    #######
     ### ##
     ##   ##
     #
     ##
`));
SHOULD_BE(JSON.stringify(m), '{"width":20,"height":22,"bpp":1,"transparent":false}');


m =  g.imageMetrics(new Uint8Array([
 8,8,1, // 8x8 1 bpp
 0,0,1,1,2,2,4,4, // 8 bits first frame
 1,1,2,2,4,4,8,8, // 8 bits second frame
 2,2,4,4,8,8,16,16, // 8 bits third frame
]));

SHOULD_BE(JSON.stringify(m), '{"width":8,"height":8,"bpp":1,"transparent":false,"frames":3}');


result = ok;
