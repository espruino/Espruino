function NRF(sck, miso, mosi, csn, ce, payload) {
  this.CSN = csn;
  this.CE = ce;
  this.PAYLOAD = payload;
  this.BASE_CONFIG = 8; //EN_CRC
  this.cmd = ""; // for receiving commands
  this.SPI = SPI1;
  this.SPI.setup({sck:SCK, miso:MISO, mosi:MOSI});
}
NRF.prototype.C = {
CONFIG      :0x00,
STATUS      :0x07,
CD          :0x09,
RX_ADDR_P0  :0x0A,
RX_ADDR_P1  :0x0B,
TX_ADDR     :0x10,
RX_PW_P0    :0x11,
RX_PW_P1    :0x12,
R_REGISTER:0x00,
W_REGISTER:0x20,
R_RX_PAYLOAD:0x61,
W_TX_PAYLOAD:0xA0,
FLUSH_TX:0xE1,
FLUSH_RX:0xE2 };
NRF.prototype.init = function(rxAddr, txAddr) {
  digitalWrite(this.CE,0);
  digitalWrite(this.CSN,1);
  this.setRXAddr(rxAddr);
  this.setTXAddr(txAddr);
  this.setReg(this.C.RX_PW_P0, this.PAYLOAD);
  this.setReg(this.C.RX_PW_P1, this.PAYLOAD);
  this.setReg(this.C.CONFIG, this.BASE_CONFIG | 2/*PWR_UP*/ | 1/*PRIM_RX*/); // RX mode
  digitalWrite(this.CE,1); // set active
}
NRF.prototype.setReg = function(reg, value) {
    this.SPI.send([this.C.W_REGISTER | reg, value], this.CSN);
}
NRF.prototype.setAddr = function(reg, value /* 5 byte array*/) {
    value = value.clone();
    value.splice(0,0,this.C.W_REGISTER | reg);
    this.SPI.send(value, this.CSN);
}
NRF.prototype.setRXAddr = function(adr /* 5 byte array*/) {
  this.setAddr(this.C.RX_ADDR_P1,adr);
}
NRF.prototype.setTXAddr = function(adr /* 5 byte array*/) {
  this.setAddr(this.C.RX_ADDR_P0,adr);
  this.setAddr(this.C.TX_ADDR,adr);
}
NRF.prototype.getReg = function(reg) {
    return this.SPI.send([this.C.R_REGISTER | reg, 0], this.CSN)[1];
}
NRF.prototype.getAddr = function(reg) {
     var data = this.SPI.send([this.C.R_REGISTER | reg, 0,0,0,0,0], this.CSN);
     data.splice(0,1); // remove first
     return data;
}
NRF.prototype.getStatus = function(reg) {
    return this.getReg(this.C.STATUS);
}
NRF.prototype.dataReady = function() {
  return (this.getReg(this.C.STATUS)&14/*RX_P_NO*/)!=14; // next payload
}
NRF.prototype.getData = function() {
  var data = [this.C.R_RX_PAYLOAD];
  for (var i=0;i<this.PAYLOAD;i++) data.push(0);
  data = this.SPI.send(data, this.CSN); // RX_DR bit
  data.splice(0,1); // remove first
  this.setReg(this.C.STATUS, 64/*RX_DR*/); // clear rx flag
  return data;
}
NRF.prototype.send = function(data/* array of length PAYLOAD */) {
  this.setReg(this.C.STATUS, 16/*MAX_RT*/|32/*TX_DS*/); // clear flags
  digitalWrite(this.CE,0); // disable
  this.setReg(this.C.CONFIG, this.BASE_CONFIG | 2/*PWR_UP*/ ); // Set TX mode
  this.SPI.send(this.C.FLUSH_TX, this.CSN);
  data = data.clone();
  data.splice(0,0,this.C.W_TX_PAYLOAD);
  this.SPI.send(data, this.CSN);
  digitalWrite(this.CE,1); // enable
  var n = 1000;
  while ((n--) && !(this.getReg(this.C.STATUS)&(16/*MAX_RT*/|32/*TX_DS*/))); // waiting
  if (n<=0) print("TX timeout");
  var success = true;
  if (this.getReg(this.C.STATUS)&16/*MAX_RT*/) {
    print("TX not received "+this.getReg(this.C.STATUS));
    success = false;
  }
  digitalWrite(this.CE,0); // disable
  this.setReg(this.C.CONFIG, this.BASE_CONFIG | 2/*PWR_UP*/ | 1/*PRIM_RX*/); // RX mode
  digitalWrite(this.CE,1); // enable
  this.setReg(this.C.STATUS, 16/*MAX_RT*/|32/*TX_DS*/); // clear flags
  return success;
}

NRF.prototype.slaveHandler = function() {
  while (this.dataReady()) {
    data = this.getData();
    for (var i in data) {
      var ch = data[i];
      if (ch==0 && this.cmd!="") {
        var c = this.cmd;
        this.cmd = "";
        print("...>"+c);
        var result = ""+eval(c); // evaluate
        print("...="+result);
        setTimeout(function() {
          this.sendString(result); // send the result back
        }, 500); // wait
      } else if (ch!=0) {
        this.cmd += String.fromCharCode(ch);
      }
    }
  }
}
NRF.prototype.masterHandler = function() {
  while (this.dataReady()) {
    data = this.getData();
    for (var i in data) {
      var ch = data[i];
      if (ch==0 && this.cmd!="") {
        print(this.cmd);
        this.cmd = "";
      } else if (ch!=0) {
        this.cmd += String.fromCharCode(ch);
      }
    }
  }
}

NRF.prototype.sendString = function(cmd) {
  for (var i=0;i<=cmd.length;i+=this.PAYLOAD) {
    var data = [];
    for (var n=0;n<this.PAYLOAD;n++) data[n] = Integer.valueOf(cmd[i+n]);
    var tries = 3;
    while ((tries-- > 0) && !this.send(data));
  }
}

// 'master' device (on Espruino)
var nrf = new NRF( A5, A6, A7, C4, C5, 4 );
function onInit() {
  nrf.init([0,0,0,0,2], [0,0,0,0,1]);
}
onInit();
setInterval("nrf.masterHandler()",50);
nrf.sendString("1+2");
// 'slave' device (on Olimexino)
var nrf = new NRF( D13, D12, D11, D1, D0, 4 );
function onInit() {
  nrf.init([0,0,0,0,1], [0,0,0,0,2]);
}
onInit();
setInterval("nrf.slaveHandler()",50);


