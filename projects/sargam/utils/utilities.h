
#ifndef realisim_utils_utilities_hh
#define realisim_utils_utilities_hh

#include <QByteArray>
class QString;

namespace realisim 
{
namespace utils 
{

  QByteArray fromFile(const QString&);
  QString getAssetFolder();
  //QString getGuid();
  //QString getMacAddress();
  bool toFile(const QString&, const QByteArray&);

}
}
#endif

