
/*function swap16(val) {
        return ((val & 0xFF) << 8)  | ((val >> 8) & 0xFF) ;
}*/
function swap16(a){return(a&255)<<8|a>>8&255}

var r = swap16(1);
result = r==256;
