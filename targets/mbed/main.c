#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"

// error handler for pure virtual calls
void __cxa_pure_virtual() { while (1); }

int main() {
  jshInit();
  jsvInit();
  jsiInit(true);
  
  while (1) 
    jsiLoop();

  // js*Kill()
}
