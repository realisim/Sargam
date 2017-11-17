

#ifndef realisim_sargam_updater_hh
#define realisim_sargam_updater_hh

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <vector>

namespace realisim
{
namespace sargam
{
    
    class Updater : public QObject
    {
        Q_OBJECT
    public:
        Updater(QObject* = 0);
        virtual ~Updater();
        
        void checkForUpdate();
        QString getDownloadPage() const;
        int getNumberOfVersions() const;
        QString getReleaseNotes(int) const;
        Version getVersion(int) const;
        QString getVersionAsQString(int) const;
        void tickRemoteCounter();
        
    signals:
        void updateInformationAvailable();
        
        protected slots:
        void handleVersionUpdates(QNetworkReply*);
        void handleTickRemoteCounter(QNetworkReply*);
        
    protected:
        struct VersionInfos
        {
            VersionInfos(QString v, QString r) : mVersion(v), mReleaseNotes(r){}
            QString mVersion;
            QString mReleaseNotes;
        };
        
        QStringList fetchTagContent(QString, QString) const;
        
        //data
        QNetworkAccessManager* mpUpdateAccess;
        QNetworkAccessManager* mpTickCounterAccess;
        std::vector<VersionInfos> mVersions;
        QString mDownloadPage;
    };

}
}


#endif
