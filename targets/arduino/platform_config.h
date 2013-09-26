#define ARDUINO
#define HAS_STDLIB


// SYSTICK is the counter that counts up and that we use as the real-time clock
// The smaller this is, the longer we spend in interrupts, but also the more we can sleep!
#define SYSTICK_RANGE 0x1000000 // the Maximum (it is a 24 bit counter) - on Olimexino this is about 0.6 sec
#define SYSTICKS_BEFORE_USB_DISCONNECT 2

#define DEFAULT_BUSY_PIN_INDICATOR (Pin)-1 // no indicator
#define DEFAULT_SLEEP_PIN_INDICATOR (Pin)-1 // no indicator

// When to send the message that the IO buffer is getting full
#define IOBUFFER_XOFF ((TXBUFFERMASK)*6/8)
// When to send the message that we can start receiving again
#define IOBUFFER_XON ((TXBUFFERMASK)*3/8)

  #define RAM_TOTAL                      8*1024
  #define IOBUFFERMASK                   31 // (max 255)
  #define TXBUFFERMASK                   31
  #define DEFAULT_CONSOLE_DEVICE         EV_SERIAL1
  #define USARTS 1
  #define SPIS 1
  #define ADCS 1

  #define BTN_PININDEX 0
  #define BTN_ONSTATE 1

