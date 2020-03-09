// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON/stringify
var obj = {
    data: 'data',
    
    toJSON : function(key) {
        if (key)
            return `Now I am a nested object under key '${key}'`;
        else
            return this;
    }
};

var pass = 0;

if (JSON.stringify(obj) == '{"data":"data"}') pass |= 1;

if (JSON.stringify({ obj:obj })=='{"obj":"Now I am a nested object under key \'obj\'"}') pass |= 2;

if (JSON.stringify([ obj ])=='["Now I am a nested object under key \'0\'"]') pass |= 4;

result = pass== 0b111;
