// Tests optional chaining with array access

var a = undefined;

result = a?.b[0] ?? true;

result |= a?.[0] ?? true;