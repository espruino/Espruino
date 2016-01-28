// Copyright 2015 by Thorsten von Eicken
// Asynchronous helpers

exports.sequence = function(funcs, scope, end) {
  scope = scope||{};
  scope.next = function() {
    //console.log("SEQ: next of", funcs.length);
    funcs.length > 0 ? funcs.shift().apply(scope, arguments) : end.call(scope);
  };
  scope.end = function() { funcs=[]; scope.next(); };
  scope.next();
}

// sequence: creates a sequence of tasks, each of which ends up calling next() to invoke the next one.
// From: http://forum.espruino.com/conversations/260159/
//exports.sequence = function(funcs,scope,done) {
//  var _seq = 0,stopped = false;
//  this.scope = scope;
//  this.next = function(){this.goto(_seq,arguments);};
//  this.goto = function(seq,args){
//    if(seq < funcs.length){
//      if(!stopped){ funcs[seq](args);_seq = seq + 1;} }
//    else{done(args);}
//  };
//  this.exit = function(){stopped = true;done(arguments);};
//  this.next();
//};

// concurrence: creates parallel tasks, all of which end up calling next().
exports.concurrence = function(funcs,scope,done) {
  var _num = funcs.length;
  this.scope = scope;
  this.next = function(){ if (--_num <= 0) done();}
  funcs.every(function(i){i();});
}; 
