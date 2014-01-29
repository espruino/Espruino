// check for multiple comments one after the other
r = eval("/* Hello */// DEF\n42");
result = r==42;
