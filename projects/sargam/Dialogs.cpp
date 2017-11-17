
#include "Dialogs.h"
#include "MainDialog.h"
#include "PartitionViewer.h"
#include "Updater.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFont>
#include <QLabel>
#include <qlayout.h>
#include <QPrinterInfo>
#include <QPushButton>
#include <QSpinBox>

using namespace realisim;
using namespace sargam;
using namespace std;

//-----------------------------------------------------------------------------
// --- PreferencesDialog
//-----------------------------------------------------------------------------
PreferencesDialog::PreferencesDialog(const MainDialog* ipMd,
                                     const PartitionViewer* ipPv,
                                     const Composition* ipC,
                                     QWidget* ipParent) :
QDialog(ipParent),
mpMainDialog(ipMd),
mpPartViewer(ipPv),
mpComposition(ipC),
mPartPreviewData(),
mpPartPreview(0)
{
    
    initUi();
    
    mPartPreviewData.setTitle( "Untitled" );
    //voir constructeur composition.
    //La composition a toujours 3 barres... gamme, tuning et une barre
    //vide. Donc la premiere barre, qui n'est pas de description, est a l'index 2.
    const int bi = 2; //barIndex
    //--- 16 notes
    mPartPreviewData.addNote(bi, Note(nvMa, -1));
    mPartPreviewData.addNote(bi, Note(nvPa, -1));
    mPartPreviewData.addNote(bi, Note(nvDha, -1));
    mPartPreviewData.addNote(bi, Note(nvNi, -1));
    mPartPreviewData.addNote(bi, Note(nvSa, 0));
    mPartPreviewData.addNote(bi, Note(nvRe, 0));
    mPartPreviewData.addNote(bi, Note(nvGa, 0));
    mPartPreviewData.addNote(bi, Note(nvMa, 0));
    mPartPreviewData.addNote(bi, Note(nvPa, 0));
    mPartPreviewData.addNote(bi, Note(nvDha, 0));
    mPartPreviewData.addNote(bi, Note(nvNi, 0));
    mPartPreviewData.addNote(bi, Note(nvSa, 1));
    mPartPreviewData.addNote(bi, Note(nvRe, 1));
    mPartPreviewData.addNote(bi, Note(nvGa, 1));
    mPartPreviewData.addNote(bi, Note(nvMa, 1));
    mPartPreviewData.addNote(bi, Note(nvPa, 1));
    
    // 4 matras a 4 notes par matra
    vector<int> m1(4, 0); m1[0] = 0; m1[1] = 1; m1[2] = 2; m1[3] = 3;
    vector<int> m2(4, 0); m2[0] = 4; m2[1] = 5; m2[2] = 6; m2[3] = 7;
    vector<int> m3(4, 0); m3[0] = 8; m3[1] = 9; m3[2] = 10; m3[3] = 11;
    vector<int> m4(4, 0); m4[0] = 12; m4[1] = 13; m4[2] = 14; m4[3] = 15;
    mPartPreviewData.addMatra(bi, m1);
    mPartPreviewData.addMatra(bi, m2);
    mPartPreviewData.addMatra(bi, m3);
    mPartPreviewData.addMatra(bi, m4);
    
    // quelques notes de grace sur les 2iem et 3ieme matra
    mPartPreviewData.addGraceNote(bi, 4);
    mPartPreviewData.addGraceNote(bi, 5);
    mPartPreviewData.addGraceNote(bi, 6);
    mPartPreviewData.addGraceNote(bi, 8);
    mPartPreviewData.addGraceNote(bi, 9);
    mPartPreviewData.addGraceNote(bi, 10);
    
    //krintan et meend sur 2ieme et 3ieme matra
    vector<NoteLocator> o1; o1.resize(4);
    o1[0] = NoteLocator(bi, 4); o1[1] = NoteLocator(bi, 5);
    o1[2] = NoteLocator(bi, 6); o1[3] = NoteLocator(bi, 7);
    vector<NoteLocator> o2; o2.resize(4);
    o2[0] = NoteLocator(bi, 8); o2[1] = NoteLocator(bi, 9);
    o2[2] = NoteLocator(bi, 10); o2[3] = NoteLocator(bi, 11);
    mPartPreviewData.addOrnement(otKrintan, o1);
    mPartPreviewData.addOrnement(otMeend, o2);
    
    //des strokes
    mPartPreviewData.addStroke(bi, stDa, 0);
    mPartPreviewData.addStroke(bi, stRa, 1);
    mPartPreviewData.addStroke(bi, stDa, 2);
    mPartPreviewData.addStroke(bi, stRa, 3);
    
    mPartPreviewData.addStroke(bi, stDa, 4);
    mPartPreviewData.addStroke(bi, stDa, 8);
    
    mPartPreviewData.addStroke(bi, stDa, 12);
    mPartPreviewData.addStroke(bi, stRa, 13);
    mPartPreviewData.addStroke(bi, stDa, 14);
    mPartPreviewData.addStroke(bi, stRa, 15);
    
    //un peu de texte
    mPartPreviewData.setBarText(bi, "Test string");
    
    mpPartPreview->setComposition( &mPartPreviewData );
    updateUi();
}
//-----------------------------------------------------------------------------
PreferencesDialog::~PreferencesDialog()
{
    if(mpPartPreview)
    {
        delete mpPartPreview;
        mpPartPreview = 0;
    }
}
//-----------------------------------------------------------------------------
void PreferencesDialog::fillPageSizeCombo()
{
    mpPageSizeCombo->clear();
    mAvailablePageSizeIds.clear();
    //ajout de letter par defaut...
    mAvailablePageSizeIds.push_back( QPageSize::Letter );
    
    QList<QPrinterInfo> pis = QPrinterInfo::availablePrinters();
    for( int i = 0; i < pis.size(); ++i )
    {
        QList<QPageSize> lps = pis.at(i).supportedPageSizes();
        for( int j = 0; j < lps.size(); ++j )
        { mAvailablePageSizeIds.push_back( lps.at(j).id() ); }
    }
    
    sort( mAvailablePageSizeIds.begin(), mAvailablePageSizeIds.end() );
    mAvailablePageSizeIds.erase( unique( mAvailablePageSizeIds.begin(),
                                        mAvailablePageSizeIds.end() ), mAvailablePageSizeIds.end() );
    
    for( int i = 0; i < mAvailablePageSizeIds.size(); ++i )
    { mpPageSizeCombo->insertItem(i, QPageSize::name( mAvailablePageSizeIds[i] ) ); }
}
//-----------------------------------------------------------------------------
realisim::sargam::Composition::Options PreferencesDialog::getCompositionOptions() const
{
    Composition::Options r;
    r.mShowLineNumber = mpShowLineNumber->isChecked();
    return r;
}

//-----------------------------------------------------------------------------
int PreferencesDialog::getFontSize() const
{ return mpFontSize->value(); }
//-----------------------------------------------------------------------------
QPageLayout::Orientation PreferencesDialog::getPageLayout() const
{
    QPageLayout::Orientation o;
    if( mpOrientation->checkedId() == 0 ){ o = QPageLayout::Portrait; }
    else{ o = QPageLayout::Landscape; }
    return o;
}
//-----------------------------------------------------------------------------
QPageSize::PageSizeId PreferencesDialog::getPageSizeId() const
{ return mAvailablePageSizeIds[ mpPageSizeCombo->currentIndex() ]; }
//-----------------------------------------------------------------------------
realisim::sargam::script PreferencesDialog::getScript() const
{ return (realisim::sargam::script)mpScriptCombo->currentIndex(); }
//-----------------------------------------------------------------------------
void PreferencesDialog::initUi()
{
    mpPartPreview = new PartitionViewer( this );
    mpPartPreview->hide();
    
    QVBoxLayout *pVBoxLayout = new QVBoxLayout(this);
    {
        QTabWidget *pTabWidget = new QTabWidget(this);
        
        QWidget *pGeneralTab = makeGeneralTab(pTabWidget);
        QWidget *pCompositionTab = makeCompositionTab(pTabWidget);
        
        pTabWidget->addTab(pGeneralTab, "General");
        pTabWidget->addTab(pCompositionTab, "Composition");
        
        //--- ok, Cancel
        QHBoxLayout* pBottomButLyt = new QHBoxLayout();
        {
            QPushButton* pOk = new QPushButton( "Ok", this );
            connect( pOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
            
            QPushButton* pCancel = new QPushButton( "Cancel", this );
            connect( pCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
            
            pBottomButLyt->addStretch(1);
            pBottomButLyt->addWidget(pOk);
            pBottomButLyt->addWidget(pCancel);
        }
        
        pVBoxLayout->addWidget(pTabWidget);
        pVBoxLayout->addLayout(pBottomButLyt);
    }
}
//-----------------------------------------------------------------------------
bool PreferencesDialog::isToolBarVisible() const
{ return mpShowToolBarChkBx->isChecked(); }
//-----------------------------------------------------------------------------
bool PreferencesDialog::isVerbose() const
{ return mpVerboseChkBx->isChecked(); }

//-----------------------------------------------------------------------------
QWidget* PreferencesDialog::makeCompositionTab(QWidget *ipParent)
{
    QWidget *pTab = new QWidget(ipParent);
    QVBoxLayout* pMainLyt = new QVBoxLayout( pTab );
    {
        mpShowLineNumber = new QCheckBox("Show line number", pTab);
        mpShowLineNumber->setChecked( mpComposition->getOptions().mShowLineNumber );
        
        pMainLyt->addWidget(mpShowLineNumber);
        pMainLyt->addStretch(1);
    }
    return pTab;
}

//-----------------------------------------------------------------------------
QWidget* PreferencesDialog::makeGeneralTab(QWidget *ipParent)
{
    QWidget *pTab = new QWidget(ipParent);
    QVBoxLayout* pMainLyt = new QVBoxLayout( pTab );
    {
        QGroupBox* pVisualizationGrp = new QGroupBox( "Visualization", this );
        {
            QVBoxLayout* pVLyt = new QVBoxLayout();
            {
                //--- font size
                QHBoxLayout *pFontSizeLyt = new QHBoxLayout();
                {
                    QLabel *pName = new QLabel("Font size:", pTab);
                    mpFontSize = new QSpinBox(pTab);
                    mpFontSize->setMinimum(10);
                    mpFontSize->setMaximum(60);
                    
                    pFontSizeLyt->addWidget(pName);
                    pFontSizeLyt->addWidget(mpFontSize);
                    
                    mpFontSize->setValue( mpPartViewer->getFontSize() );
                }
                
                //--- script
                QHBoxLayout *pScriptLyt = new QHBoxLayout();
                {
                    QLabel *pName = new QLabel("Script:", pTab);
                    mpScriptCombo = new QComboBox(pTab);
                    mpScriptCombo->insertItem(sLatin, "Latin");
                    mpScriptCombo->insertItem(sDevanagari, "देवनागरी");
                    
                    pScriptLyt->addWidget(pName);
                    pScriptLyt->addWidget(mpScriptCombo);
                    
                    mpScriptCombo->setCurrentIndex( mpPartViewer->getScript() );
                }
                
                //--- part preview
                QHBoxLayout *pPartPreviewLyt = new QHBoxLayout();
                {
                    mpPreviewLabel = new QLabel("n/a", pTab);
                    
                    pPartPreviewLyt->addStretch(1);
                    pPartPreviewLyt->addWidget(mpPreviewLabel, Qt::AlignHCenter);
                    pPartPreviewLyt->addStretch(1);
                }
                
                //--- taille du papier
                QHBoxLayout* pPaperLyt = new QHBoxLayout();
                {
                    QLabel* pLabel = new QLabel( "Page size: ", pTab );
                    
                    mpPageSizeCombo = new QComboBox(pTab);
                    
                    pPaperLyt->addWidget(pLabel);
                    pPaperLyt->addWidget(mpPageSizeCombo);
                    
                    fillPageSizeCombo();
                    int currrentIndex = distance( mAvailablePageSizeIds.begin(),
                                                 std::find( mAvailablePageSizeIds.begin(), mAvailablePageSizeIds.end(),
                                                           mpPartViewer->getPageSizeId() ) );
                    mpPageSizeCombo->setCurrentIndex( currrentIndex );
                }
                
                //--- Portrait, landscape
                QHBoxLayout* pOrientationLyt = new QHBoxLayout();
                {
                    mpOrientation = new QButtonGroup( pTab );
                    mpPortrait = new QCheckBox( "Portrait", pTab );
                    mpLandscape = new QCheckBox( "Landscape", pTab );
                    mpOrientation->addButton( mpPortrait, 0 );
                    mpOrientation->addButton( mpLandscape, 1 );
                    
                    pOrientationLyt->addStretch(1);
                    pOrientationLyt->addWidget(mpPortrait);
                    pOrientationLyt->addWidget(mpLandscape);
                    
                    QPageLayout::Orientation o = mpPartViewer->getLayoutOrientation();
                    if( o == QPageLayout::Portrait )
                    { mpPortrait->setCheckState( Qt::Checked ); }
                    else{ mpLandscape->setCheckState( Qt::Checked ); }
                }
                
                connect(mpFontSize, SIGNAL(valueChanged(int)),
                        this, SLOT(updateUi()));
                connect(mpScriptCombo, SIGNAL(activated(int)),
                        this, SLOT(updateUi()));
                
                pVLyt->addLayout(pFontSizeLyt);
                pVLyt->addLayout(pScriptLyt);
                pVLyt->addLayout(pPartPreviewLyt);
                pVLyt->addLayout( pPaperLyt );
                pVLyt->addLayout( pOrientationLyt );
            } //vLyt
            
            pVisualizationGrp->setLayout(pVLyt);
        } //groupbox
        
        //--- log
        mpVerboseChkBx = new QCheckBox( "Verbose log", pTab);
        mpVerboseChkBx->setChecked(mpMainDialog->isVerbose());
        
        //--- show tool bar
        mpShowToolBarChkBx = new QCheckBox( "Show toolbar", pTab );
        mpShowToolBarChkBx->setChecked(mpMainDialog->isToolBarVisible());
        
        pMainLyt->addWidget( pVisualizationGrp );
        pMainLyt->addWidget( mpVerboseChkBx );
        pMainLyt->addWidget( mpShowToolBarChkBx );
        pMainLyt->addStretch(1);
    }
    return pTab;
}

//-----------------------------------------------------------------------------
void PreferencesDialog::updateUi()
{
    mpPartPreview->setFontSize( mpFontSize->value() );
    mpPartPreview->setScript( (script)mpScriptCombo->currentIndex() );
    QPixmap pix = QPixmap::fromImage(mpPartPreview->getBarAsImage(2));
    mpPreviewLabel->setPixmap(pix);
}

//------------------------------------------------------------------------------
//--- SaveDialog
//------------------------------------------------------------------------------
SaveDialog::SaveDialog(QWidget *ipParent, QString iFileName) :
QDialog(ipParent),
mAnswer(aCancel),
mFileName(iFileName)
{ createUi(); }
//------------------------------------------------------------------------------
void SaveDialog::cancel()
{
    mAnswer = aCancel;
    reject();
}
//------------------------------------------------------------------------------
void SaveDialog::createUi()
{
    QVBoxLayout *pVlyt = new QVBoxLayout(this);
    {
        QLabel *pLabel = new QLabel(this);
        QString t;
        t.sprintf("Do you want to save the changes you made to %s?",
                  mFileName.toStdString().c_str() );
        pLabel->setText(t);
        QFont f = pLabel->font();
        f.setBold(true);
        pLabel->setFont( f );
        
        QLabel *pLabel2 = new QLabel(this);
        pLabel2->setText("Your changes will be lost if you don't save them.");
        
        //-- buttons
        QHBoxLayout *pButtonsLyt = new QHBoxLayout();
        {
            QPushButton *pDontSave = new QPushButton("Don't save", this);
            connect( pDontSave, SIGNAL(clicked()), this, SLOT(dontSave()) );
            
            QPushButton *pSave = new QPushButton("Save", this);
            connect( pSave, SIGNAL(clicked()), this, SLOT(save()) );
            
            QPushButton *pCancel = new QPushButton("Cancel", this);
            connect( pCancel, SIGNAL(clicked()), this, SLOT(cancel()) );
            
            pButtonsLyt->addWidget(pDontSave);
            pButtonsLyt->addStretch(1);
            pButtonsLyt->addWidget(pCancel);
            pButtonsLyt->addWidget(pSave);
        }
        
        pVlyt->addWidget(pLabel);
        pVlyt->addWidget(pLabel2);
        pVlyt->addLayout(pButtonsLyt);
    }
}
//------------------------------------------------------------------------------
void SaveDialog::dontSave()
{
    mAnswer = aDontSave;
    accept();
}
//------------------------------------------------------------------------------
void SaveDialog::save()
{
    mAnswer = aSave;
    accept();
}
//------------------------------------------------------------------------------
SaveDialog::answer SaveDialog::getAnswer() const
{ return mAnswer; }

//------------------------------------------------------------------------------
//--- UpdateDialog
//------------------------------------------------------------------------------
UpdateDialog::UpdateDialog(QWidget *ipParent, QString iCurrentVersion,
                           const Updater *ipUpdater) :
QDialog(ipParent),
mCurrentVersion(iCurrentVersion),
mpUpdater(ipUpdater)
{ createUi(); }
//------------------------------------------------------------------------------
void UpdateDialog::createUi()
{
    setWindowFlags(Qt::Dialog);
    
    resize(540, 440);
    setWindowTitle("New version available");
    setWindowModality(Qt::ApplicationModal);
    
    QVBoxLayout *pVlyt = new QVBoxLayout(this);
    pVlyt->setMargin(0); pVlyt->setSpacing(2);
    {
        QTextEdit *pTextEdit = new QTextEdit(this);
        pTextEdit->setReadOnly(true);
        
        //get all release notes
        QString t;
        for( int i = 0; i < mpUpdater->getNumberOfVersions(); ++i )
        {
            if( mCurrentVersion < mpUpdater->getVersionAsQString(i) )
            {
                t += mpUpdater->getReleaseNotes(i);
                t += "<hr>";
            }
        }
        t += "You are currently using version: " + mCurrentVersion + "<br>";
        pTextEdit->setText(t);
        
        
        //add cancel and visit web site button
        QHBoxLayout *pButLyt = new QHBoxLayout();
        {
            QPushButton *pCancel = new QPushButton("Cancel", this);
            connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()) );
            
            QPushButton *pVisitWebSite = new QPushButton("Visit web site", this);
            connect(pVisitWebSite, SIGNAL(clicked()), this, SLOT(accept()) );
            
            pButLyt->addStretch(1);
            pButLyt->addWidget(pCancel);
            pButLyt->addWidget(pVisitWebSite);
        }
        
        pVlyt->addWidget(pTextEdit);
        pVlyt->addLayout(pButLyt);
    }
}