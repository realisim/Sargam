

#ifndef MainDialog_hh
#define MainDialog_hh

#include "data.h"
#include "Dialogs.h"
#include <QProxyStyle>
#include <QPrinter>
#include <QtWidgets>
#include <QSettings>
#include "utils/Log.h"
#include "Updater.h"
#include <vector>
#include <map>

namespace realisim { namespace sargam { class PartitionViewer; } }

class CustomProxyStyle : public QProxyStyle
{
public:
    CustomProxyStyle() : QProxyStyle() {}
    
    virtual void drawPrimitive(PrimitiveElement,
                               const QStyleOption*, QPainter*, const QWidget* = 0) const;
protected:
};

//------------------------------------------------------------------------------
class MainDialog : public QMainWindow
{
    Q_OBJECT
public:
    MainDialog();
    ~MainDialog(){};
    
    const realisim::utils::Log& getLog() const {return mLog;}
    QString getVersionAsQString() const;
    realisim::sargam::Version getVersion() const {return mVersion;}
    bool isToolBarVisible() const {return mIsToolBarVisible;}
    bool isVerbose() const;
    void setAsVerbose( bool );
    
    protected slots:
    void about();
    void ensureVisible( QPoint );
    void generatePrintPreview(QPrinter*);
    void handleUpdateAvailability();
    void newFile();
    void openFile();
    void preferences();
    void print();
    void printPreview();
    void redoActivated();
    void save();
    void saveAs();
    void toggleDebugging();
    void toggleLogTiming();
    void toolActionTriggered(QAction*);
    void undoActivated();
    void updateUi();
    
protected:
    enum state{ sNormal, sUpdatesAreAvailable };
    enum action{ aAddBar, aLineJump, aAddMatra, aRemoveMatra, aAddKrintan,
        aAddMeend, aAddGamak, aAddAndolan, aRemoveOrnement, aAddGraceNote, aRemoveGraceNote,
        aAddParenthesis, aRemoveParenthesis,
        aDecreaseOctave, aIncreaseOctave, aRest, aChik, aPhrasing, aTivra, aShuddh, aKomal,
        aDa, aRa, aDiri, aRemoveStroke, aUnknown };
    
    void applyPrinterOptions( QPrinter* );
    void closeEvent( QCloseEvent* );
    void createUi();
    void createToolBar();
    action findAction( QAction* ) const;
    bool hasSaveFilePath() const;
    state getState() const;
    void loadSettings();
    void fillPageSizeCombo( QComboBox* );
    QString getSaveFilePath() const;
    QString getSaveFileName() const;
    realisim::sargam::SaveDialog::answer saveIfNeeded();
    void saveSettings();
    void setState(state);
    void setToolBarVisible( bool i ) { mIsToolBarVisible = i; }
    void showUpdateDialog();
    void updateActions();
    
    QScrollArea* mpScrollArea;
    realisim::sargam::PartitionViewer* mpPartitionViewer;
    QToolBar* mpToolBar;
    std::map< action, QAction* > mActions;
    realisim::sargam::Updater* mpUpdater;
    
    realisim::sargam::Version mVersion;
    QSettings mSettings;
    QString mSaveFilePath;
    QString mLastSavePath;
    realisim::sargam::Composition mComposition;
    realisim::utils::Log mLog;
    bool mIsVerbose;
    bool mIsToolBarVisible;
    state mState;
};


#endif
