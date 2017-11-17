
#include "utilities.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QString>

namespace realisim
{
namespace utils
{
  //-----------------------------------------------------------------------------
  QByteArray fromFile(const QString& iFile)
  {
    QByteArray r;
    QFile f(iFile);
    if(f.open(QIODevice::ReadOnly))
    {
      r = f.readAll();
      f.close();
    }
    return r;
  }
  
  //-----------------------------------------------------------------------------
  //Retourne le path vers le repertoire assets qui se trouve generalement
  //au meme niveau que l'application livrée. Par contre lors du developpement
  //l'application est souvent dans un repertoire Debug/Release et sous mac
  //l'application est dans un bundle...
  //Retourne QString null si pas trouvé
  QString getAssetFolder()
  {
    QDir appDirPath(QCoreApplication::applicationDirPath());
    bool found = false;
    while( !found && appDirPath.cdUp()  )
    {
      QStringList childs = appDirPath.entryList(QDir::Dirs);
      found = childs.indexOf("assets") != -1;
    }
    
    return found ? appDirPath.absolutePath() + "/assets/" : QString();
  }
  
  //a deplacer dans netWorkutils...
  //-----------------------------------------------------------------------------
  //QString getGuid()
  //{
  //	QString r;
  //  r = getMacAddress();
  //  if( !r.isEmpty() )
  //  { r += QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss:zzz"); }
  //  return r;
  //}
  //-----------------------------------------------------------------------------
  //QString getMacAddress()
  //{
  //  foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
  //  {
  //			if( !(netInterface.flags() & QNetworkInterface::IsLoopBack) &&
  //      	!netInterface.hardwareAddress().isEmpty() )
  //      { return netInterface.hardwareAddress(); }
  //  }
  //  return QString();
  //}
  //-----------------------------------------------------------------------------
  bool toFile(const QString& iFile , const QByteArray& iData)
  {
    bool r = false;
    QFile f(iFile);
    if(f.open(QIODevice::WriteOnly))
    {
      qint64 bytesWritten = f.write(iData.constData(), iData.size());
      if(bytesWritten == iData.size())
        r = true;
      f.close();
    }
    return r;
  }

  
}
}