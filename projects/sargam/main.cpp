
#include <iostream>
#include "MainDialog.h"
#include <QApplication>

int startMainApp()
{
    if ( qApp )
    {
        return qApp->exec();
    }
    else
    {
        printf("ERROR: unable to start Sargam\n");
        return 0;
    }
}

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  app.setStyle(new CustomProxyStyle());
  MainDialog m;
  m.show();

  try
  {
    if ( startMainApp() == 0 )
    {
      //we are closing!
    }
  }
  catch(...)
  {
    printf("oups! uncaught exception\n");
  }
  

  return 0;
}
