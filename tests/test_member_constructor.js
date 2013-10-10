// Test that object member functions can be used as constructors
var hw = { setup_function : function() { this.hw_foo = 1; } };
result = (new hw.setup_function()).hw_foo;
