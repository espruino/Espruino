/*
  Espruino.h
*/

// ensure this library description is only included once
#ifndef Espruino_h
#define Espruino_h

// library interface description
class Espruino
{
  // user-accessible "public" interface
  public:
    Espruino();
    void init(void);
    void loop(void);

};

#endif

