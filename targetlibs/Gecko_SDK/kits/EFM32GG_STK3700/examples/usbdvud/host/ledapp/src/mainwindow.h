#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qled.h"

namespace Ui {
    class MainWindow;
}

class Led;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void VendorUniqueSetLedCmd( int ledNo, int on );
    void UpdateLedsOnForm( void );
    uint8_t VendorUniqueGetLedsCmd( void );
    void ConnectUSBDevice( void );
    void DisconnectUSBDevice( void );

    void LedClicked( int ledNo );
    Led *Led0,*Led1,*Led2,*Led3,*Led4;

private:
    Ui::MainWindow *ui;
};

class Led : public QLed
{
  Q_OBJECT
  public:
    int ledNo;
    MainWindow *parentForm;
    Led( QWidget *parent = 0,
         int ledno = 0,
         int x=0, int y=0, int width=1, int height=1 ) : QLed( parent )
    {
      QString s = "Led";
      parentForm = (MainWindow*)parent;
      ledNo = ledno;
      setGeometry(QRect(x, y, width, height));
      s.append( ledNo + '0' );
      setObjectName(s);
      setValue(false);
      setOnColor(QLed::Orange);
      setOffColor(QLed::Grey);
      setShape(QLed::Circle);
    }
  protected:
    void mousePressEvent( QMouseEvent *e );
};

#endif // MAINWINDOW_H
