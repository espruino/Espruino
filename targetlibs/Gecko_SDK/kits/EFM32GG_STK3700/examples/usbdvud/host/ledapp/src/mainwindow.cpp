#include <QMouseEvent>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lusb0_usb.h"
#include "qled.h"

#define VND_GET_LEDS 0x10
#define VND_SET_LED  0x11

static usb_dev_handle *pDevH = NULL;  // USB device handle
static struct usb_device *pDev;
static struct usb_bus *pBus;
static uint8_t leds;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  Led4 = new Led( this, 4, 20,  20, 50, 50 );
  Led3 = new Led( this, 3, 80,  20, 50, 50 );
  Led2 = new Led( this, 2, 140, 20, 50, 50 );
  Led1 = new Led( this, 1, 200, 20, 50, 50 );
  Led0 = new Led( this, 0, 260, 20, 50, 50 );

  ConnectUSBDevice();
}

MainWindow::~MainWindow()
{
  DisconnectUSBDevice();
  delete ui;
}

void MainWindow::LedClicked( int ledNo )
{
  QString s;

  if ( leds & (1<<ledNo) )  VendorUniqueSetLedCmd( ledNo, 0 );
  else                      VendorUniqueSetLedCmd( ledNo, 1 );

  UpdateLedsOnForm();
  s = "Toggled Led ";
  s.append( ledNo + '0' );
  ui->statusBar->showMessage( s );
}

void MainWindow::VendorUniqueSetLedCmd( int ledNo, int on )
{
  if ( pDevH )
  {
    usb_control_msg(
                  pDevH,            // Device handle
                  USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                                    // bmRequestType
                  VND_SET_LED,      // bRequest
                  on,               // wValue
                  ledNo,            // wIndex
                  NULL,             // char *pBuffer
                  0x0000,           // wLength
                  1000 );           // Timeout (ms)
  }
}

uint8_t MainWindow::VendorUniqueGetLedsCmd( void )
{
  uint8_t leds;

  if ( pDevH )
  {
    usb_control_msg(
                  pDevH,            // Device handle
                  USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                                    // bmRequestType
                  VND_GET_LEDS,     // bRequest
                  0,                // wValue
                  0,                // wIndex
                  (char*)&leds,     // char *pBuffer
                  1,                // wLength
                  1000 );           // Timeout (ms)
  }

  return leds;
}

void MainWindow::UpdateLedsOnForm( void )
{
  leds = VendorUniqueGetLedsCmd();

  if ( leds & 1 )   Led0->setValue( true );
  else              Led0->setValue( false );
  if ( leds & 2 )   Led1->setValue( true );
  else              Led1->setValue( false );
  if ( leds & 4 )   Led2->setValue( true );
  else              Led2->setValue( false );
  if ( leds & 8 )   Led3->setValue( true );
  else              Led3->setValue( false );
  if ( leds & 16 )  Led4->setValue( true );
  else              Led4->setValue( false );
}

void MainWindow::ConnectUSBDevice( void )
{
  if ( pDevH == NULL )
  {
    usb_find_busses();
    usb_find_devices();

    // Enumerate USB devices ...
    for ( pBus = usb_get_busses(); pBus; pBus = pBus->next)
    {
      for ( pDev = pBus->devices; pDev; pDev = pDev->next)
      {
        if ( ( ( pDev->descriptor.idVendor  == 0x2544 ) ||
               ( pDev->descriptor.idVendor  == 0x10C4 )    ) &&
             ( ( pDev->descriptor.idProduct == 0x0001 ) ||
               ( pDev->descriptor.idProduct == 0x0008 ) ||
               ( pDev->descriptor.idProduct == 0x000C )    )    )
        {
          pDevH = usb_open( pDev );
          if ( pDevH )
          {
            if ( usb_set_configuration(
                      pDevH, pDev->config->bConfigurationValue ) != 0 )
            {
              usb_close( pDevH );
              pDevH = NULL;
              goto found;
            }

            if ( pDev->descriptor.idProduct == 0x0001 )
            {
              if ( usb_claim_interface( pDevH, 0 ) != 0 )
              {
                usb_close( pDevH );
                pDevH = NULL;
                goto found;
              }
            }

            if ( pDev->descriptor.idProduct == 0x0008 )
            {
              if ( usb_claim_interface( pDevH, 0 ) != 0 )
              {
                usb_close( pDevH );
                pDevH = NULL;
                goto found;
              }
            }
          }
          goto found;
        }
      }
    }

found:
    if ( pDevH == NULL )
    {
      ui->statusBar->showMessage( "No USB device found" );
    }
    else
    {
      ui->statusBar->showMessage( "EFM32 Vendor Unique Device found, click on LED's to toggle" );
      leds = VendorUniqueGetLedsCmd();
      UpdateLedsOnForm();
    }
  }
}

void MainWindow::DisconnectUSBDevice( void )
{
  if ( pDevH )
  {
    usb_set_configuration( pDevH, 0 );
    usb_release_interface( pDevH, 0 );
    usb_close( pDevH );
    pDevH = NULL;
  }
}

void Led::mousePressEvent( QMouseEvent *e )
{
  if ( e->buttons() & Qt::LeftButton )
  {
    parentForm->LedClicked( this->ledNo );
  }
}
