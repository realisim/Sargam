
#include <algorithm>
#include <cassert>
#include "Commands.h"
#include "MainDialog.h"
#include "PartitionViewer.h"
#include "utils/utilities.h"
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QTextBoundaryFinder>
#include <QTextLayout>
#include <set>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

using namespace std;
using namespace realisim;
using namespace utils;
using namespace sargam;

namespace
{
    const int kBarFadeDuration = 1000; //msec
    
    const int kSpacing = 5;
    //page
    const int kInterPageGap = 15;
    const double kPageMarginInCm = 1.5;
    const int kPageFooter = 15;
    //bar
    const int kKrintanHeight = 6;
    const int kGamakHeight = 4;
    const int kOrnementHeight = 10;
    const int kMatraHeight = 10;
    const int kNoBarIndex = -1;
    const double kUnderineRatioLength = 0.8;
    
    
    const QString kPushButtonStyle =
    "QWidget#partitionViewer"
    "{"
    "  background-color: white;"
    "}"
    ""
    "QPushButton#partitionViewer"
    "{"
    "  border-style: solid;"
    "  border-width: 1px;"
    "  border-color: lightgrey;"
    "  border-radius: 3px;"
    "  color: lightgrey;"
    "}"
    ""
    "QPushButton#partitionViewer:hover"
    "{"
    "  border-style: solid;"
    "  border-width: 1px;"
    "  border-color: black;"
    "  border-radius: 3px;"
    "  color: black;"
    "}"
    ""
    "QPushButton#partitionViewer:pressed"
    "{"
    "  border-style: solid;"
    "  border-width: 1px;"
    "  border-color: black;"
    "  background-color: lightgrey;"
    "  border-radius: 3px;"
    "  color: black;"
    "}";
}

realisim::sargam::Composition PartitionViewer::mDummyComposition;
PartitionViewer::Bar PartitionViewer::mDummyBar;

//-----------------------------------------------------------------------------
// --- ThinLineEdit
//-----------------------------------------------------------------------------
ThinLineEdit::ThinLineEdit( QWidget* ipParent ) :
QLineEdit(ipParent)
{ setAttribute(Qt::WA_MacShowFocusRect, 0); }

//-----------------------------------------------------------------------------
// --- partition viewer
//-----------------------------------------------------------------------------
PartitionViewer::PartitionViewer( QWidget* ipParent ) :
QWidget( ipParent ),
mpTitleEdit(0),
mpLineTextEdit(0),
mDebugMode( dmNone ),
mPageSizeId( QPageSize::Letter ),
mLayoutOrientation( QPageLayout::Portrait ),
mNumberOfPages(0),
mCurrentBar( -1 ),
mCurrentBarTimerId(0),
mCurrentNote( -1 ),
mEditingBarText( -1 ),
mEditingDescriptionBarLabel(-1),
mEditingLineIndex( -1 ),
mEditingParentheseIndex( -1 ),
mEditingTitle( false ),
mBarHoverIndex( kNoBarIndex ),
mDescriptionBarLabelHoverIndex(kNoBarIndex),
x( &mDummyComposition ),
mDefaultLog(),
mpLog( &mDefaultLog ),
mIsVerbose( false ),
mHasLogTiming( false ),
mHasModifications( false )
{
    setMouseTracking( true );
    setFocusPolicy( Qt::StrongFocus );
    srand( time(NULL) );
    
    //pour que le style du QWidget s'applique
    setObjectName("PartitionViewer");
    
    setStyleSheet(kPushButtonStyle);
    
    QString fontFamily( "Arial" );
    
    //definition des tailles de polices
    float titleSize = 24;
    float barFontSize = 14;
    float barTextSize = 10;
    float graceNoteSize = 10;
    float lineFontSize = 12;
    float strokeFontSize = 10;
    float parenthesisFontSize = 9;
    
    mTitleFont = QFont( fontFamily );
    mTitleFont.setBold( true );
    mTitleFont.setPointSizeF( titleSize );
    mBarFont = QFont( fontFamily );
    mBarFont.setPointSizeF( barFontSize );
    mBarTextFont = QFont( fontFamily );
    mBarTextFont.setPointSizeF( barTextSize );
    mGraceNotesFont = QFont( fontFamily );
    mGraceNotesFont.setPointSizeF( graceNoteSize );
    mLineFont = QFont( fontFamily );
    mLineFont.setPointSizeF( lineFontSize );
    mStrokeFont = QFont( fontFamily );
    mStrokeFont.setPointSizeF( strokeFontSize );
    mParenthesisFont = QFont( fontFamily );
    mParenthesisFont.setPointSizeF( parenthesisFontSize );
    
    mBaseParenthesisRect = QRectF( QPoint(0, 0),
                                  QSize(8, QFontMetrics(mBarFont).height() * 1.8) );
    
    //initialisation du Ui et premiere mise à jour.
    createUi();
    addPage();
    updateUi();
}

PartitionViewer::~PartitionViewer()
{
    //mUndoRedoStack nettoie lui même la mémoire.
}

//-----------------------------------------------------------------------------
/*Ajoute une barre a la suite de la barre iBarIndex*/
void PartitionViewer::addBar( int iBarIndex )
{
    //on ajoute une barre pour les donnees d'affichage.
    vector<Bar>::iterator it = mBars.begin() + iBarIndex + 1;
    if( it < mBars.end() )
    { mBars.insert(it, Bar()); }
    else{ mBars.push_back( Bar() ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::addBarTextClicked()
{
    startBarTextEdit( getCurrentBar() );
}
//-----------------------------------------------------------------------------
void PartitionViewer::addDescriptionBarClicked()
{
    CommandAddDescriptionBar* c = new CommandAddDescriptionBar(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::addLineTextClicked()
{
    const int i = x->findLine(getCurrentBar());
    setBarAsDirty( i, true );
    startLineTextEdit( i );
}
//-----------------------------------------------------------------------------
void PartitionViewer::addNoteToSelection( int iBar, int iNote )
{
    mSelectedNotes.push_back( make_pair( iBar, iNote ) );
    sort( mSelectedNotes.begin(), mSelectedNotes.end() );
    /*On empeche quil y ait 2 fois la meme note dans la selection.*/
    mSelectedNotes.erase(
                         std::unique( mSelectedNotes.begin(), mSelectedNotes.end() ),
                         mSelectedNotes.end() );
    
    /*On met la barre dirty parce que le rectangle de selection est dessine
     sur la barre (voir methode renderBarOffScreen).*/
    setBarAsDirty( iBar, true );
}
//-----------------------------------------------------------------------------
void PartitionViewer::addPage()
{ setNumberOfPage( getNumberOfPages() + 1 ); }
//-----------------------------------------------------------------------------
void PartitionViewer::clear()
{
    setNumberOfPage(1);
    mBars.clear();
    mLines.clear();
    mBarsPerPage.clear();
    mOrnements.clear();
    mCurrentBar = -1;
    mCurrentNote = -1;
    mSelectedNotes.clear();
    mParenthesis.clear();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: clear." ); }
}
//-----------------------------------------------------------------------------
Note PartitionViewer::alterNoteFromScale( Note iN ) const
{
    vector<Note> v = x->getScale();
    for( int i = 0; i < v.size(); ++i )
    {
        if( v[i].getValue() == iN.getValue() )
        { iN.setModification( v[i].getModification() ); break; }
    }
    return iN;
}
//-----------------------------------------------------------------------------
void PartitionViewer::clearSelection()
{
    for( int i = 0; i < mSelectedNotes.size(); ++i )
    { setBarAsDirty( mSelectedNotes[i].first , true ); }
    mSelectedNotes.clear();
}
//-----------------------------------------------------------------------------
int PartitionViewer::cmToPixel( double iCm ) const
{ return logicalDpiX() * iCm / 2.54; }
//-----------------------------------------------------------------------------
void PartitionViewer::commandAddBar()
{
    CommandAddBar *c = new CommandAddBar(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
/*Les notes de la selection deviennent des graces notes si elle ne le sont
 pas déjà.*/
void PartitionViewer::commandAddGraceNotes()
{
    CommandAddGraceNotes *c = new CommandAddGraceNotes(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandAddLine()
{
    CommandAddLine *c = new CommandAddLine(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandAddMatra()
{
    CommandAddMatra *c = new CommandAddMatra(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandAddNote( Note iN )
{
    CommandAddNote* c = new CommandAddNote(this, iN);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandAddOrnement( ornementType iOt )
{
    CommandAddOrnement *c = new CommandAddOrnement(this, iOt);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandAddParenthesis( int iNumber )
{
    CommandAddParenthesis *c = new CommandAddParenthesis(this, iNumber);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandAddStroke( strokeType iSt )
{
    CommandAddStroke *c = new CommandAddStroke(this, iSt);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandBreakMatrasFromSelection()
{
    CommandBreakMatrasFromSelection *c = new CommandBreakMatrasFromSelection(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandBreakOrnementsFromSelection()
{
    CommandBreakOrnementsFromSelection *c = new CommandBreakOrnementsFromSelection(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandDecreaseOctave()
{
    CommandDecreaseOctave *c = new CommandDecreaseOctave(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandErase()
{
    CommandErase * c = new CommandErase(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandIncreaseOctave()
{
    CommandIncreaseOctave *c = new CommandIncreaseOctave(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandRemoveParenthesis()
{
    CommandRemoveParenthesis *c = new CommandRemoveParenthesis(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandRemoveSelectionFromGraceNotes()
{
    CommandRemoveSelectionFromGraceNotes *c = new CommandRemoveSelectionFromGraceNotes(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandRemoveStroke()
{
    CommandRemoveStroke *c = new CommandRemoveStroke(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::commandShiftNote()
{
    CommandShiftNote *c = new CommandShiftNote(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
void PartitionViewer::createUi()
{
    //titre
    /* Astuce: les line edit on comme parent le parent du widget... ainsi
     les touches pesees lors de l'édition* n'interfere jamais avec la
     méthode keyPressEvent de la classe PartitionViewer. */
    mpTitleEdit = new ThinLineEdit( this->parentWidget() );
    mpTitleEdit->setFont( mTitleFont );
    mpTitleEdit->hide();
    connect( mpTitleEdit, SIGNAL( editingFinished() ),
            this, SLOT( stopTitleEdit() ) );
    connect( mpTitleEdit, SIGNAL( textChanged(const QString&)),
            this, SLOT( resizeEditToContent() ) );
    
    //line edit pour titre des lignes
    mpLineTextEdit = new ThinLineEdit( this->parentWidget() );
    mpLineTextEdit->setFont( mLineFont );
    mpLineTextEdit->hide();
    connect( mpLineTextEdit, SIGNAL( editingFinished() ),
            this, SLOT( stopLineTextEdit() ) );
    connect( mpLineTextEdit, SIGNAL( textChanged(const QString&)),
            this, SLOT( resizeEditToContent() ) );
    
    //ligne pour le texte des barres
    mpBarTextEdit = new ThinLineEdit( this->parentWidget() );
    mpBarTextEdit->setFont( mBarTextFont );
    mpBarTextEdit->hide();
    connect( mpBarTextEdit, SIGNAL( editingFinished() ),
            this, SLOT( stopBarTextEdit() ) );
    connect( mpBarTextEdit, SIGNAL( textChanged(const QString&)),
            this, SLOT( resizeEditToContent() ) );
    
    //line edit pour les label de barres de description
    mpDescriptionBarLabelEdit = new ThinLineEdit( this->parentWidget() );
    mpDescriptionBarLabelEdit->setFont( mBarFont );
    mpDescriptionBarLabelEdit->hide();
    connect( mpDescriptionBarLabelEdit, SIGNAL( editingFinished() ),
            this, SLOT( stopDescriptionBarLabelEdit() ) );
    connect( mpDescriptionBarLabelEdit, SIGNAL( textChanged(const QString&)),
            this, SLOT( resizeEditToContent() ) );
    
    //spin box pour le nombre de répétitions des parentheses
    mpParenthesisEdit = new QSpinBox( this->parentWidget() );
    mpParenthesisEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);
    mpParenthesisEdit->setMinimum(2); mpParenthesisEdit->setMaximum(99);
    connect( mpParenthesisEdit, SIGNAL( editingFinished() ),
            this, SLOT( stopParentheseEdit() ) );
    connect( mpParenthesisEdit, SIGNAL( valueChanged(int)),
            this, SLOT( resizeEditToContent() ) );
    
    //add remove description bar
    mpAddRemoveDescriptionBar = new QWidget(this);
    mpAddRemoveDescriptionBar->setObjectName("partitionViewer");
    QHBoxLayout* pLyt = new QHBoxLayout(mpAddRemoveDescriptionBar);
    {
        pLyt->setMargin(5);
        pLyt->setSpacing(2);
        
        //button to add description bar
        QPushButton* pAdd = new QPushButton( this );
        connect(pAdd, SIGNAL(clicked()), this, SLOT(addDescriptionBarClicked()));
        pAdd->setObjectName("partitionViewer");
        pAdd->setFont(mLineFont);
        pAdd->setText("Add");
        
        //button to remove description bar
        QPushButton* pRemove = new QPushButton( this );
        connect(pRemove, SIGNAL(clicked()), this, SLOT(removeDescriptionBarClicked()));
        pRemove->setObjectName("partitionViewer");
        pRemove->setFont(mLineFont);
        pRemove->setText("Remove");
        
        pLyt->addWidget(pAdd);
        pLyt->addWidget(pRemove);
        pLyt->addStretch(1);
    }
    
    //add line text button
    mpAddLineTextButton = new QPushButton( this );
    mpAddLineTextButton->setObjectName("partitionViewer");
    mpAddLineTextButton->setFont(mLineFont);
    mpAddLineTextButton->setText(" + ");
    connect( mpAddLineTextButton, SIGNAL(clicked()),
            this, SLOT( addLineTextClicked() ) );
    
    //add line text button
    mpAddBarTextButton = new QPushButton( this );
    mpAddBarTextButton->setObjectName("partitionViewer");
    mpAddBarTextButton->setFont(mBarTextFont);
    mpAddBarTextButton->setText(" + ");
    connect( mpAddBarTextButton, SIGNAL(clicked()),
            this, SLOT( addBarTextClicked() ) );
}
//-----------------------------------------------------------------------------
/*Ajoute une barre apres la barre courante, les notes suivant le curseur sont
 ajoutées à la nouvelle barre*/
void PartitionViewer::doCommandAddBar()
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    // ajout de la barre
    int cb = getCurrentBar();
    x->addBar( cb );
    addBar( cb );
    clearSelection();
    
    //déplacement des notes dans la nouvelle barre. Le + 1 dans la boucle est pour
    //déplacer les notes qui suivent le curseur.
    vector<NoteLocator> vn;
    int moveFromIndex = getCurrentNote() + 1;
    for( int i = moveFromIndex; i < x->getNumberOfNotesInBar(cb); ++i )
    {
        vn.push_back( NoteLocator( cb, i) );
        x->addNote( cb+1, x->getNote(cb, i) );
    }
    
    //deplace les matras
    moveMatraForward( cb, moveFromIndex );
    //deplace les ornements
    moveOrnementForward( cb, moveFromIndex );
    //déplace les strokes
    moveStrokeForward( cb, moveFromIndex );
    //déplace note de grace
    moveGraceNoteForward(cb, moveFromIndex);
    //déplace parentheses
    moveParenthesisForward( cb, moveFromIndex );
    
    //efface le note de la barre courrante
    for( int i = vn.size() - 1; i >= 0; --i )
    { x->eraseNote( vn[i].getBar(), vn[i].getIndex() ); }
    
    setBarAsDirty( cb, true );
    setBarAsDirty( cb + 1, true );
    setCursorPosition( NoteLocator(cb + 1, -1) );
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandAddBar." ); }
}
//-----------------------------------------------------------------------------
/**/
void PartitionViewer::doCommandAddDescriptionBar()
{
    x->addDescriptionBar("Description");
    const int iIndex = x->getNumberOfDescriptionBars() - 1;
    
    x->addNote(iIndex, Note(nvSa));
    x->addNote(iIndex, Note(nvRe));
    x->addNote(iIndex, Note(nvGa));
    
    addBar(iIndex);
    setBarAsDirty(iIndex, true);
    setAsModified(true);
    updateUi();
}
//-----------------------------------------------------------------------------
/*Les notes de la selection deviennent des graces notes si elle ne le sont
 pas déjà.*/
void PartitionViewer::doCommandAddGraceNotes()
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    if( hasSelection() )
    {
        for( int i = 0; i < mSelectedNotes.size(); ++i )
        {
            int barIndex = mSelectedNotes[i].first;;
            int noteIndex = mSelectedNotes[i].second;
            x->addGraceNote( barIndex, noteIndex );
            setBarAsDirty( barIndex, true );
        }
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandAddGraceNotes." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandAddLine()
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    doCommandAddBar();
    x->addLine( getCurrentBar() );
    setBarAsDirty( getCurrentBar(), true );
    setAsModified(true);
    setCursorPosition( NoteLocator(getCurrentBar(), -1) );
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandAddLine." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandAddMatra()
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    if( hasSelection() )
    {
        /*On commence par chercher si il y a deja un matra associe aux
         note du matra qu'on est en train d'ajouter. Si oui, on enleve le matra
         deja existants.*/
        for( int i = 0; i < mSelectedNotes.size(); ++i )
        {
            int bar = mSelectedNotes[i].first;
            int noteIndex = mSelectedNotes[i].second;
            if( x->isNoteInMatra( bar, noteIndex) )
            {
                x->eraseMatra( bar, x->findMatra( bar, noteIndex ) );
                setBarAsDirty( bar, true );
            }
        }
        
        map<int, vector<int> > m = splitPerBar( mSelectedNotes );
        map<int, vector<int> >::const_iterator it = m.begin();
        for( ; it != m.end(); ++it )
        {
            x->addMatra( it->first, it->second );
            setBarAsDirty( it->first, true );
        }
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandAddMatra." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandAddNote( Note iN )
{
    if( hasSelection() )
    { doCommandErase(); }
    Note n = alterNoteFromScale( iN );
    x->addNote( getCurrentBar(), getCurrentNote(), n );
    setCursorPosition( NoteLocator(getCurrentBar(), getCurrentNote()+1) );
    clearSelection();
    setBarAsDirty(getCurrentBar(), true);
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandAddNote." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandAddOrnement( ornementType iOt )
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    if( hasSelection() )
    {
        /*On commence par chercher si il y a deja un ornement associe aux
         note de lornement qu'on est en train d'ajouter. Si oui, on enleve lornement
         deja existants.*/
        for( int i = 0; i < mSelectedNotes.size(); ++i )
        {
            int bar = mSelectedNotes[i].first;
            int noteIndex = mSelectedNotes[i].second;
            if( x->isNoteInOrnement( bar, noteIndex) )
            { eraseOrnement( x->findOrnement( bar, noteIndex ) ); }
        }
        
        mOrnements.push_back( Ornement() );
        x->addOrnement( iOt, toNoteLocator( mSelectedNotes ) );
        setBarsAsDirty( x->getBarsInvolvedByOrnement(
                                                     x->findOrnement( mSelectedNotes[0].first, mSelectedNotes[0].second) ), true );
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandAddOrnement." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandAddParenthesis( int iNumber )
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    if( hasSelection() )
    {
        /*On commence par chercher si il y a deja une parenthese associée aux
         note de la parenthese qu'on est en train d'ajouter. Si oui, on enleve la
         parenthese deja existante.*/
        for( int i = 0; i < mSelectedNotes.size(); ++i )
        {
            int bar = mSelectedNotes[i].first;
            int noteIndex = mSelectedNotes[i].second;
            int p = x->findParenthesis( bar, noteIndex );
            if( p != -1 )
            { x->eraseParenthesis( p ); }
        }
        
        x->addParenthesis( toNoteLocator( mSelectedNotes ) );
        
        //dirty sur toutes les barres impliquées.
        map< int, vector<int> > m = splitPerBar( mSelectedNotes );
        map< int, vector<int> >::const_iterator it = m.begin();
        for( ; it != m.end(); ++it )
        { setBarAsDirty(it->first, true); }
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: docommandAddParenthesis." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandAddStroke( strokeType iSt )
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    vector< pair<int, int> > v;
    if( hasSelection() )
    {
        for( int i = 0; i < mSelectedNotes.size(); ++i )
        { v.push_back( mSelectedNotes[i] ); }
    }
    else{ v.push_back( make_pair( getCurrentBar(), getCurrentNote() ) ); }
    
    for( int i = 0; i < v.size(); ++i )
    {
        int bar = v[i].first;
        int index = v[i].second;
        
        /*On commence par chercher si il y a deja un stroke associe aux
         notes du stroke qu'on est en train d'ajouter. Si oui, on enleve les strokes
         deja existants.*/
        if( x->hasStroke( bar, index ) )
        {
            x->eraseStroke( bar, x->findStroke( bar, index ) );
            setBarAsDirty( bar, true );
        }
    }
    
    //on ajoute les strokes par barre
    map< int, vector<int> > m = splitPerBar( v );
    map< int, vector<int> >::const_iterator it = m.begin();
    for( ; it != m.end(); ++it )
    {
        int bar = it->first;
        x->addStroke( bar, iSt, it->second );
        setBarAsDirty( bar, true );
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandAddStroke." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandBreakMatrasFromSelection()
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    for( int i = 0; i < mSelectedNotes.size(); ++i )
    {
        int bar = mSelectedNotes[i].first;
        int index = mSelectedNotes[i].second;
        int matraIndex = x->findMatra( bar, index );
        if( matraIndex != -1 )
        {
            x->eraseMatra( bar, matraIndex );
            setBarAsDirty( bar, true );
        }
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandBreakMatrasFromSelection." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandBreakOrnementsFromSelection()
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    for( int i = 0; i < mSelectedNotes.size(); ++i )
    {
        int bar = mSelectedNotes[i].first;
        int index = mSelectedNotes[i].second;
        int ornementIndex = x->findOrnement( bar, index );
        if( ornementIndex != -1 )
        { eraseOrnement( ornementIndex ); }
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandBreakOrnementsFromSelection." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandDecreaseOctave()
{
    vector< pair<int, int> > v;
    if( hasSelection() )
    {
        for( int i = 0; i < mSelectedNotes.size(); ++i )
        { v.push_back( mSelectedNotes[i] ); }
    }
    else{ v.push_back( make_pair( getCurrentBar(), getCurrentNote() ) ); }
    
    for( int i = 0; i < v.size(); ++i )
    {
        Note n = x->getNote( v[i].first, v[i].second );
        n.setOctave( n.getOctave() - 1 );
        x->setNote( v[i].first, v[i].second, n );
        setBarAsDirty( v[i].first, true );
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCcommandDecreaseOctave." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandErase()
{
    if( hasSelection() )
    {
        /*On efface les notes en partant de la fin parce que la selection contient
         les indices des notes, ainsi en commençant par la fin, nous n'avons pas à
         ajuster les indices.*/
        for( int i = mSelectedNotes.size() - 1; i >= 0; --i )
        {
            int bar = mSelectedNotes[i].first;
            int index = mSelectedNotes[i].second;
            if(index != -1)
            { x->eraseNote( bar, index ); }
            else
            {
                //quand on efface la note -1, il faut enlever la barre
                //et potentiellement la ligne
                /*On efface la ligne si la note courante est -1 et que la barre est
                 un début de ligne. Par contre, on ne peut pas effacer la ligne 0.*/
                if( x->isStartOfLine( bar ) && x->findLine( bar ) != 0 )
                { x->eraseLine( x->findLine( bar ) ); }
                /*On ne peut pas effacer les barres de descriptions, ni la premiere barre
                 qui suit les descriptions...*/
                if( bar > x->getNumberOfDescriptionBars() )
                { eraseBar( bar ); }
            }
            
            if( x->isNoteInOrnement(bar, index) )
            {
                setBarsAsDirty( x->getBarsInvolvedByOrnement(
                                                             x->findOrnement(bar, index) ), true );
            }
        }
        
        setCursorPosition( getPrevious( NoteLocator(mSelectedNotes[0].first, mSelectedNotes[0].second) ) );
        clearSelection();
    }
    else
    {
        int cb = getCurrentBar();
        if( getCurrentNote() != -1 )
        {
            x->eraseNote( cb, getCurrentNote() );
            setCursorPosition( NoteLocator(cb, getCurrentNote() - 1) );
        }
        else
        {
            /*On efface la ligne si la note courante est -1 et que la barre est
             un début de ligne. Par contre, on ne peut pas effacer la ligne 0.*/
            if( x->isStartOfLine( cb ) && x->findLine( cb ) != 0 )
            { x->eraseLine( x->findLine( cb ) ); }
            /*On ne peut pas effacer les barres de descriptions, ni la premiere barre
             qui suit les descriptions...*/
            if( cb > x->getNumberOfDescriptionBars() )
            {
                int i = x->getNumberOfNotesInBar( cb - 1 ) - 1;
                eraseBar( cb );
                setCursorPosition( NoteLocator(cb - 1, i) );
            }
        }
        setBarAsDirty( getCurrentBar(), true );
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandErase." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandIncreaseOctave()
{
    vector< pair<int, int> > v;
    if( hasSelection() )
    {
        for( int i = 0; i < mSelectedNotes.size(); ++i )
        { v.push_back( mSelectedNotes[i] ); }
    }
    else{ v.push_back( make_pair( getCurrentBar(), getCurrentNote() ) ); }
    
    for( int i = 0; i < v.size(); ++i )
    {
        Note n = x->getNote( v[i].first, v[i].second );
        n.setOctave( n.getOctave() + 1 );
        x->setNote( v[i].first, v[i].second, n );
        setBarAsDirty( v[i].first, true );
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandIncreaseOctave." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandRemoveDescriptionBar()
{
    const int index = x->getNumberOfDescriptionBars() - 1;
    
    if( x->eraseDescriptionBar( index ) )
    {
        //efface des données d'affichage
        if( index >= 0 && index < x->getNumberOfBars() )
        {
            mBars.erase( mBars.begin() + index );
            if( x->getNumberOfBars() > 0 ){ setBarAsDirty(0, true); }
        }
    }
    
    setAsModified(true);
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandRemoveParenthesis()
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    /*On commence par chercher si il y a deja une parenthese associée aux
     note de la parenthese qu'on est en train d'ajouter. Si oui, on enleve la
     parenthese deja existante.*/
    for( int i = 0; i < mSelectedNotes.size(); ++i )
    {
        int bar = mSelectedNotes[i].first;
        int noteIndex = mSelectedNotes[i].second;
        int p = x->findParenthesis( bar, noteIndex );
        if( p != -1 )
        { x->eraseParenthesis( p ); setBarAsDirty( bar, true ); }
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandRemoveParenthesis." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandRemoveSelectionFromGraceNotes()
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    for( int i = 0; i < mSelectedNotes.size(); ++i )
    {
        int bar = mSelectedNotes[i].first;
        int index = mSelectedNotes[i].second;
        if( x->isGraceNote( bar, index ) )
        {
            x->eraseGraceNote( bar, index );
            setBarAsDirty( bar, true );
        }
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandRemoveSelectionFromGraceNotes." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandRemoveStroke()
{
    /*Cette commande ne peut pas etre executée sur les barres speciales*/
    if( getCurrentBar() < x->getNumberOfDescriptionBars() ){ return; }
    
    vector< pair<int, int> > l;
    if( hasSelection() )
    { l = mSelectedNotes; }
    else
    { l.push_back( make_pair( getCurrentBar(), getCurrentNote()) ); }
    
    for( int i = 0; i < l.size(); ++i )
    {
        int bar = l[i].first;
        int index = l[i].second;
        int strokeIndex = x->findStroke( bar, index );
        if( strokeIndex != -1 )
        {
            x->eraseStroke( bar, strokeIndex );
            setBarAsDirty( bar, true );
        }
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandRemoveStroke." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::doCommandShiftNote()
{
    vector< pair<int, int> > l;
    if( hasSelection() )
    { l = mSelectedNotes; }
    else
    { l.push_back( make_pair( getCurrentBar(), getCurrentNote()) ); }
    
    for( int i = 0; i < l.size(); ++i )
    {
        int bar = l[i].first;
        int index = l[i].second;
        Note n = x->getNote( bar, index );
        if( n.getModification() == nmKomal || n.getModification() == nmTivra )
        { n.setModification( nmShuddh ); }
        else if( n.canBeKomal() )
        { n.setModification( nmKomal); }
        else if( n.canBeTivra() )
        { n.setModification( nmTivra ); }
        x->setNote( bar, index, n );
        setBarAsDirty( bar, true );
    }
    setAsModified(true);
    updateUi();
    
    if( isVerbose() )
    { getLog().log( "PartitionViewer: doCommandShiftNote." ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::draw( QPaintDevice* iPaintDevice, QRect iPaintRegion ) const
{
    QElapsedTimer _timer;
    _timer.start();
    
    QPainter p( iPaintDevice );
    p.setBackgroundMode( Qt::OpaqueMode );
    QBrush b( Qt::white );
    QPen pen = p.pen();
    p.setBackground( b );
    
    //on fait le rendu uniquement des barres qui sont sur des pages
    //visible
    for(int i = 0; i < getNumberOfPages(); ++i )
    {
        if( getPageRegion(prPage, i).intersects( iPaintRegion ) )
        {
            //on nettoie la page en dessinant un rectangle blanc
            //on dessine le background blanc de la page
            p.setPen( Qt::white );
            p.setBrush( Qt::white );
            p.drawRect( getPageRegion( prPage, i ) );
            
            //--- tout le text de la page
            p.setPen( Qt::black );
            p.setBrush( Qt::NoBrush );
            
            if( i == 0) //premiere page
            {
                //--- titre
                drawTitle( &p );
                
                //--- rendu des textes pour barres de description
                drawDescriptionBars( &p );
            }
            
            //render bars
            vector<int> bars = getBarsFromPage(i);
            for( int j = 0; j < (int)bars.size(); ++j )
            {
                drawBar( &p, bars[j] );
                if( x->isStartOfLine( bars[j] ) )
                { drawLine( &p, x->findLine( bars[j] ) ); }
            }
        }
    }
    
    drawCursor( &p );
    drawSelectedNotes( &p );
    
    //on dessine le contour de la barre courante
    if( mCurrentBarTimerId != 0 )
    {
        QColor c = getColor( cSelection );
        c.setAlpha( 255 - 255 *
                   min( mCurrentBarTimer.elapsed()/(double)kBarFadeDuration, 1.0) );
        drawBarContour( &p, getCurrentBar(), c );
    }
    
    drawPageFooters( &p );
    
    if( hasLogTiming() )
    { getLog().log("PartitionViewer::draw: %.3f ms", _timer.nsecsElapsed() / 1000000.0 ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawBar( QPainter* iP, int iBar ) const
{
    iP->save();
    
    const Bar& b = getBar( iBar );
    if( !b.mIsWayTooLong )
    {
        QPoint topLeft = b.mScreenLayout.topLeft();
        QTransform transfo;
        transfo.translate( topLeft.x(), topLeft.y() );
        iP->setWorldTransform( transfo, true );
        
        iP->setRenderHints( iP->renderHints() | QPainter::Antialiasing );
        QPen pen = iP->pen();
        pen.setColor( Qt::black );
        
        //les mots qui contiennent les notes
        for(size_t j = 0; j < b.mWords.size(); ++j)
        {
            QFont f = b.mWords[j].second ? mGraceNotesFont : mBarFont;
            iP->setFont(f);
            iP->drawText( b.mWordLayouts[j].bottomLeft() -
                         QPointF(0, QFontMetricsF(f).descent()),
                         b.mWords[j].first );
        }
        
        //les decoration d'octave tivra et de komal
        for(size_t j = 0; j < x->getNumberOfNotesInBar(iBar); ++j )
        {
            Note n = x->getNote(iBar, j);
            
            /*Le script devanagari demande une barre sous la note Komal*/
            if( getScript() == sDevanagari && n.getModification() == nmKomal )
            {
                barRegion br = x->isGraceNote(iBar, j) ? brGraceNoteKomalLineY :
                brKomalLineY;
                
                int y = getBarRegion(br, b.mBarType);
                
                const QRectF noteRect = b.getNoteRect(j);
                double e = noteRect.width() - noteRect.width() * kUnderineRatioLength;
                float xStart = noteRect.left() + e / 2.0;
                float xEnd = noteRect.right() - e / 2.0;
                iP->save();
                QPen pen = iP->pen();
                pen.setWidth( 1 );
                iP->setPen(pen);
                iP->drawLine(QPointF(xStart, y), QPointF(xEnd, y));
                iP->restore();
            }
            
            /*l'ocatave superieur (1) et représenté par un point sur la note.
             l'octave inférieur (-1) est représenté par un point sous la note.*/
            if( n.getOctave() != 0 )
            {
                barRegion br = n.getOctave() == 1 ? brUpperOctaveY : brLowerOctaveY;
                if(x->isGraceNote(iBar, j))
                { br = n.getOctave() == 1 ? brGraceNoteUpperOctaveY : brGraceNoteLowerOctaveY; }
                int y = getBarRegion(br, b.mBarType);
                float x = b.getNoteRect(j).left() + b.getNoteRect(j).width() / 2.0;
                iP->save();
                QPen pen = iP->pen();
                pen.setWidth( 3 );
                pen.setCapStyle(Qt::RoundCap);
                iP->setPen(pen);
                iP->drawPoint( QPointF(x,y) );
                iP->restore();
            }
        }
        
        //la barre vertical à la fin de la barre
        if( b.mBarType == Bar::btNormal )
        {
            int x1 = b.mRect.right();
            int y1 = 0.15 * getBarHeight(b.mBarType);
            int y2 = getBarRegion( brStrokeY );
            iP->drawLine( x1, y1, x1, y2 );
            
            //--- render beat groups
            for( int i = 0; i < b.mMatraGroupsRect.size(); ++i )
            {
                QRect r = b.mMatraGroupsRect[i];
                iP->drawArc( r, -10 * 16, -170 * 16 );
            }
            
            //--- render Ornements: meends and krintans
            for( int i = 0; i < x->getNumberOfOrnements(); ++i )
            {
                if( x->ornementAppliesToBar( i, iBar ) )
                {
                    const Ornement& m = mOrnements[i];
                    QRect r =  m.mFullOrnement;
                    
                    iP->save();
                    iP->setClipRect( QRect( QPoint(0, 0), b.mScreenLayout.size() ) );
                    iP->translate( m.getOffset( iBar ), 0 );
                    iP->translate( m.getDestination( iBar ), 0 );
                    ornementType ot = x->getOrnementType(i);
                    switch (ot)
                    {
                        case otMeend:
                        { iP->drawArc( r, 10 * 16, 170 * 16 ); } break;
                        case otKrintan:
                        {
                            iP->drawLine( r.topLeft() + QPointF(0, kKrintanHeight), r.topLeft() );
                            iP->drawLine( r.topLeft(), r.topRight() );
                            iP->drawLine( r.topRight() + QPointF(0, kKrintanHeight), r.topRight() );
                        } break;
                        case otAndolan:
                        {
                            r.adjust(0, (r.height() - kGamakHeight) / 2,
                                     0, -(r.height() - kGamakHeight) / 2);
                            drawGamak( iP, r, 2 );
                        } break;
                        case otGamak:
                        {
                            r.adjust(0, (r.height() - kGamakHeight) / 2,
                                     0, -(r.height() - kGamakHeight) / 2);
                            drawGamak( iP, r, 3 );
                        } break;
                        default:break;
                    }
                    
                    iP->restore();
                }
            }
            
            //render strokes
            iP->save();
            iP->setFont(mStrokeFont);
            for( int i = 0; i < x->getNumberOfStrokesInBar(iBar); ++i )
            {
                QRectF r, r2;
                for( int j = 0; j < x->getNumberOfNotesInStroke(iBar, i); ++j )
                {
                    int noteIndex = x->getNoteIndexFromStroke(iBar, i, j);
                    if( r.isNull() )
                    { r = b.getNoteRect( noteIndex ); }
                    else { r = r.united( b.getNoteRect( noteIndex ) ); }
                }
                r2.setTop( getBarRegion( brStrokeY ) );
                r2.setLeft( r.left() );
                r2.setWidth( r.width() );
                r2.setHeight( QFontMetrics(mBarFont).height() );
                QString a = strokeToString( x->getStrokeType(iBar, i) );
                iP->drawText( r2, Qt::AlignCenter, a );
            }
            iP->restore();
            
            //render text si pas en edition.
            if( mEditingBarText != iBar )
            {
                iP->save();
                iP->setFont(mBarTextFont);
                iP->drawText( b.mTextRect.bottomLeft(), x->getBarText(iBar) );
                iP->restore();
            }
            
            //render Parenthesis
            iP->save();
            iP->setFont( mParenthesisFont );
            for( int i = 0; i < x->getNumberOfParenthesis(); ++i )
            {
                if( x->parenthesisAppliesToBar(i, iBar) )
                {
                    const Parenthesis& p = mParenthesis[i];
                    NoteLocator first = x->getNoteLocatorFromParenthesis(i, 0);
                    NoteLocator last = x->getNoteLocatorFromParenthesis(i,
                                                                        x->getNumberOfNotesInParenthesis(i) - 1);
                    if( first.getBar() == iBar )
                    { iP->drawArc( p.mOpening, 90 * 16, 169 * 16 ); }
                    if( last.getBar() == iBar )
                    {
                        iP->drawArc( p.mClosing, 80 * 16, -169 * 16 );
                        iP->drawText( p.mText.bottomLeft(), getParenthesisText(i) );
                    }
                }
            }
            iP->restore();
        } //fin de if( b.mBarType == Bar::btNormal )
    }
    else
    {
        iP->save();
        iP->resetTransform();
        QPen pen = iP->pen();
        pen.setStyle( Qt::DashLine );
        pen.setColor( Qt::red );
        iP->setPen( pen );
        iP->drawRoundedRect(b.mScreenLayout, 2, 2);
        iP->drawText(b.mScreenLayout, Qt::AlignCenter, "Multi-lines bar are not "
                     "supported yet...");
        iP->restore();
    }
    
    iP->restore();
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawBarContour( QPainter* iP, int iBarIndex, QColor iCol ) const
{
    if( iBarIndex >= 0 && hasFocus() )
    {
        QPen pen = iP->pen();
        iP->save();
        iP->setRenderHints( QPainter::Antialiasing );
        pen.setColor( iCol );
        pen.setWidth( 1 );
        iP->setPen( pen );
        const Bar& b = getBar( iBarIndex );
        iP->drawRoundedRect( b.mScreenLayout, 2, 2 );
        iP->restore();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawCursor( QPainter* iP ) const
{
    QPen pen = iP->pen();
    //le curseur dans la barre courante s'il n'y a pas de selection
    //et que le focus y est
    pen.setColor( Qt::black );
    pen.setWidth( 1 );
    iP->setPen( pen );
    if( !hasSelection() && hasFocus() )
    { iP->drawLine( getCursorLine() ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawGamak( QPainter* iP, QRect iR, double iNumPi ) const
{
    iP->save();
    iP->translate( iR.topLeft() );
    QFontMetrics fm(mBarFont);
    QPainterPath pp;
    //20 point par 2*Pi.
    //iNumPi par 2 notes -> iNumPi/2 par notes
    //combien de pi à faire...
    const double numPointsPer2Pi = 20.0;
    const double pi = 3.1415629;
    const float nbPi = iR.width() * iNumPi/2.0 / fm.width("S ");
    const double numIter = nbPi * numPointsPer2Pi / 2.0;
    const double incPerIter = 2*pi / numPointsPer2Pi;
    double eval = 0;
    for( double i = 0; i < numIter; i ++, eval += incPerIter )
    { pp.lineTo( i / numIter * iR.width(), -sin(eval)/2 * iR.height() ); }
    iP->drawPath( pp );
    iP->restore();
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawLine( QPainter* iP, int iLine ) const
{
    QPen pen = iP->pen();
    
    pen.setColor( Qt::black );
    iP->setPen( pen );
    iP->setFont( mLineFont );
    const Line& l = mLines[ iLine ];
    
    //numero de ligne. les lignes commencent a 1!!!
    if(x->getOptions().mShowLineNumber)
    {
        iP->drawText( l.mLineNumberRect.bottomLeft(),
                     QString::number( iLine + 1 ) + ")" );
    }
    
    //text de la ligne
    if( mEditingLineIndex != iLine )
    { iP->drawText( l.mTextScreenLayout.bottomLeft(), x->getLineText(iLine) ); }
    
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawPageFooter( QPainter* iP, int i ) const
{
    //on dessine le bas de page
    //l'indice de page commence à 0, mais on veut que montrer page 1.
    QString text;
    text.sprintf( "%s: %d out of %d", x->getTitle().toStdString().c_str(),  i + 1,
                 getNumberOfPages() );
    iP->setFont(QFont("Arial", 10));
    iP->setPen( Qt::gray );
    iP->drawText( getPageRegion( prPageFooter, i ), Qt::AlignCenter, text );
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawPageFooters( QPainter* iP ) const
{
    for( int i = 0; i < getNumberOfPages(); ++i )
    { drawPageFooter( iP, i ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawSelectedNotes( QPainter* iP ) const
{
    iP->save();
    iP->setRenderHints( iP->renderHints() | QPainter::Antialiasing );
    
    map<int, vector<int> > m = splitPerBar(mSelectedNotes);
    map<int, vector<int> >::iterator it = m.begin();
    for( ; it != m.end(); ++it )
    {
        QRectF r;
        const Bar& b = getBar( it->first );
        const vector<int>& vn = it->second;
        for( int i = 0 ; i < vn.size(); ++i )
        {
            int noteIndex = vn[i];
            if( r.isNull() )
            { r = b.getNoteRect(noteIndex); }
            else{ { r = r.united(b.getNoteRect(noteIndex)); } }
        }
        
        QTransform transfo;
        QPoint topLeft = b.mScreenLayout.topLeft();
        transfo.translate( topLeft.x(), topLeft.y() );
        iP->setWorldTransform( transfo );
        
        QPen pen = iP->pen();
        pen.setColor( getColor( cSelection ) );
        iP->setPen( pen );
        if( !r.isNull() )
        {
            r.adjust( -2, -2, 2, 2 );
            iP->drawRoundedRect( r, 2, 2 );
        }
    }
    
    iP->restore();
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawDescriptionBars( QPainter* iP ) const
{
    iP->save();
    iP->setFont( mBarFont );
    for(int i = 0; i < x->getNumberOfDescriptionBars(); ++i)
    {
        //draw description bar label
        if( mEditingDescriptionBarLabel != i )
        {
            iP->drawText( getRegion( dbrLabel, i ), Qt::AlignCenter, x->getLabelFromDescriptionBar(i) );
        }
        drawBar(iP, i);
    }
    iP->restore();
}
//-----------------------------------------------------------------------------
void PartitionViewer::drawTitle( QPainter* iP ) const
{
    iP->save();
    
    if( !hasTitleEditionPending() )
    {
        iP->setFont( mTitleFont );
        iP->setPen( Qt::black );
        iP->drawText( mTitleScreenLayout.bottomLeft(), x->getTitle() );
    }
    
    iP->restore();
}
//-----------------------------------------------------------------------------
/*Efface la barre iBar des donnees et des donnees d'affichage. Si la barre
 contient des notes, ces notes sont transférées à la barre précédente
 
 Yark! le contenu de cette fonction devrait être dans la classe
 composition.
 */
void PartitionViewer::eraseBar( int iBar )
{
    int moveToIndex = x->getNumberOfNotesInBar( iBar - 1 );
    //transfer des notes dans la barre précédente
    for( int i = 0; i < x->getNumberOfNotesInBar( iBar ); ++i )
    { x->addNote(iBar - 1, x->getNote( iBar, i ) ); }
    
    //on met la barre precedente comme dirty
    setBarAsDirty(iBar - 1, true);
    
    //transfert des matra
    moveMatraBackward(iBar, moveToIndex);
    //transfert des ornements
    moveOrnementBackward(iBar, moveToIndex);
    //transfert des strokes
    moveStrokeBackward(iBar, moveToIndex);
    //transfert des notes de grace
    moveGraceNoteBackward(iBar, moveToIndex);
    //transfert des parenthses
    moveParenthesisBackward(iBar, moveToIndex);
    
    //efface des données d'affichage
    if( iBar >= 0 && iBar < x->getNumberOfBars() )
    {
        mBars.erase( mBars.begin() + iBar );
        if( x->getNumberOfBars() > 0 ){ setBarAsDirty(0, true); }
    }
    //efface les données
    x->eraseBar( iBar );
}
//-----------------------------------------------------------------------------
/*Efface l'ornement iIndex de données d'affichage et des données.*/
void PartitionViewer::eraseOrnement( int iIndex )
{
    if( iIndex >= 0 && iIndex < mOrnements.size() )
    {
        setBarsAsDirty( x->getBarsInvolvedByOrnement( iIndex ), true );
        mOrnements.erase( mOrnements.begin() + iIndex );
        x->eraseOrnement( iIndex );
    }
}
//-----------------------------------------------------------------------------
PartitionViewer::Bar& PartitionViewer::getBar( int iBar )
{
    return const_cast<Bar&>(
                            const_cast< const PartitionViewer* >(this)->getBar(iBar) );
}
//-----------------------------------------------------------------------------
const PartitionViewer::Bar& PartitionViewer::getBar( int iBar ) const
{
    const Bar* r = &mDummyBar;
    
    if( iBar >= 0 && iBar < (int)mBars.size() )
    { r = &mBars[iBar]; }
    return *r;
}
//-----------------------------------------------------------------------------
QImage PartitionViewer::getBarAsImage(int iBar) const
{
    QRect rect = getBar(iBar).mScreenLayout;
    QImage im(size(), QImage::Format_ARGB32);
    im.fill(Qt::white);
    QPainter p(&im);
    drawBar(&p, iBar);
    p.end();
    im = im.copy(rect);
    return im;
}
//-----------------------------------------------------------------------------
vector<int> PartitionViewer::getBarsFromPage( int iPage ) const
{
    vector<int> r;
    if( iPage >= 0 && iPage < getNumberOfPages() )
    { r = mBarsPerPage.at(iPage); }
    return r;
}
//-----------------------------------------------------------------------------
int PartitionViewer::getBarHeight(Bar::barType iT) const
{
    QFontMetrics fm(mBarFont);
    QFontMetrics btfm(mBarTextFont);
    int r = 60;
    switch (iT) {
        case Bar::btNormal: r = 4*fm.height() + btfm.height(); break;
        case Bar::btDescription: r = 1.5*fm.height(); break;
        default: break;
    }
    return r;
}
//-----------------------------------------------------------------------------
int PartitionViewer::getBarRegion( barRegion br, Bar::barType iBt ) const
{
    
    /*
     |    text
     |    ornement
     |    upperOctave
     |    Notes
     |    Komal line - devanagari only
     |    lowerOctave
     |    matra
     |    stroke
     
     */
    
    int r = 0;
    const QFontMetrics fm(mBarFont);
    const QFontMetrics gnfm(mGraceNotesFont);
    const QFontMetrics barTextFm(mBarTextFont);
    
    const int kKomalSpacing = getScript() == sDevanagari ? fm.descent() : 0;
    const int kKomalGraceNoteSpacing = getScript() == sDevanagari ? gnfm.descent() : 0;
    const int kLowerOctaveSpacing = getScript() == sDevanagari ? 4 : 1;
    
    switch( br )
    {
        case brTextY: r = 2; break;
        case brOrnementY: r = getBarRegion(brGraceNoteTopY, iBt) - kOrnementHeight ; break;
        case brUpperOctaveY: r = getBarRegion(brNoteTopY, iBt) ; break;
        case brGraceNoteUpperOctaveY: r = getBarRegion(brGraceNoteTopY, iBt) ; break;
        case brNoteTopY: r = getBarHeight(iBt) / 2 - fm.height() / 2; break;
        case brGraceNoteTopY:
            r = getBarRegion( brNoteTopY, iBt ) - gnfm.height() / 4; break;
        case brGraceNoteBottomY:
            r = getBarRegion( brGraceNoteTopY, iBt ) + gnfm.height(); break;
        case brNoteBottomY: r = getBarHeight(iBt) / 2 + fm.height() / 2; break;
        case brKomalLineY:
            r = getBarRegion(brNoteBottomY, iBt) + kKomalSpacing;
            break;
        case brGraceNoteKomalLineY:
            r = getBarRegion(brGraceNoteBottomY, iBt) + kKomalGraceNoteSpacing;
            break;
        case brGraceNoteLowerOctaveY:
            r = getBarRegion(brGraceNoteBottomY, iBt) + kKomalGraceNoteSpacing + kLowerOctaveSpacing; break;
        case brLowerOctaveY:
            r = getBarRegion(brNoteBottomY, iBt) + kKomalSpacing + kLowerOctaveSpacing; break;
        case brMatraGroupY: r = getBarRegion(brLowerOctaveY, iBt) ; break;
        case brStrokeY: r = getBarRegion(brMatraGroupY, iBt) + kMatraHeight; break;
            
        case brNoteStartX: r = fm.width(" "); break;
        case brTextX: r = getBarRegion(brNoteStartX, iBt); break;
            
        default: break;
    }
    return r;
}
//-----------------------------------------------------------------------------
QColor PartitionViewer::getColor( colors iC ) const
{
    QColor r;
    switch( iC )
    {
        case cHover: r = QColor( 125, 125, 125, 120 ); break;
        case cSelection: r = QColor( 0, 10, 210, 120 ); break;
        default: break;
    }
    
    return r;
}
//-----------------------------------------------------------------------------
Composition PartitionViewer::getComposition() const
{ return *x; }
//-----------------------------------------------------------------------------
QLineF PartitionViewer::getCursorLine() const
{
    QLineF l;
    const Bar& b = getBar( getCurrentBar() );
    const int cn = getCurrentNote();
    if( cn >= 0 && cn < b.mNoteScreenLayouts.size() )
    {
        QRectF r = b.mNoteScreenLayouts[getCurrentNote()];
        l.setPoints( r.topRight(), r.bottomRight() );
    }
    else
    {
        int x = b.mScreenLayout.left() + getBarRegion( brNoteStartX );
        int y = b.mScreenLayout.top() + getBarRegion( brNoteTopY, b.mBarType );
        int h = getBarRegion( brNoteBottomY ) - getBarRegion( brNoteTopY );
        l.setPoints( QPoint(x,y) , QPoint( x, y + h ) );
    }
    return l;
}
//-----------------------------------------------------------------------------
int PartitionViewer::getCurrentBar() const
{ return mCurrentBar; }
//-----------------------------------------------------------------------------
int PartitionViewer::getCurrentNote() const
{ return mCurrentNote; }
//-----------------------------------------------------------------------------
NoteLocator PartitionViewer::getCursorPosition() const
{ return NoteLocator( mCurrentBar, mCurrentNote ); }
//-----------------------------------------------------------------------------
PartitionViewer::debugMode PartitionViewer::getDebugMode() const
{ return mDebugMode; }
//-----------------------------------------------------------------------------
int PartitionViewer::getFontSize() const
{ return mBarFont.pointSize(); }
//-----------------------------------------------------------------------------
QPageLayout::Orientation PartitionViewer::getLayoutOrientation() const
{ return mLayoutOrientation; }
//-----------------------------------------------------------------------------
const Log& PartitionViewer::getLog() const
{ return *mpLog; }
//-----------------------------------------------------------------------------
Log& PartitionViewer::getLog()
{ return const_cast<Log&>( const_cast< const PartitionViewer* >(this)->getLog() ); }
//-----------------------------------------------------------------------------
QString PartitionViewer::getInterNoteSpacingAsQString( NoteLocator n1, NoteLocator n2 ) const
{
    QString r;
    
    int matra1 = x->findMatra( n1.getBar(), n1.getIndex() );
    int matra2 = x->findMatra( n2.getBar(), n2.getIndex() );
    //les notes ne sont pas du meme matra
    if( matra1 == -1 || matra2 == -1  || matra1 != matra2 )
    { r = " "; }
    
    //si on passe de note de grace a note normale et vice versa
    if( x->isGraceNote( n1.getBar(), n1.getIndex() ) !=
       x->isGraceNote( n2.getBar(), n2.getIndex() ) )
    { r = " "; }
    
    //les répetitions
    /*On ajoute 2 ou 3 espaces entre les notes separé par une parenthèse */
    int p1 = x->findParenthesis( n1.getBar(), n1.getIndex() );
    int p2 = x->findParenthesis( n2.getBar(), n2.getIndex() );
    if( p1 == -1 && p2 != -1 ) //ouverture
    { r = "  "; }
    if( p1 != -1 && p2 == -1 ) //fermeture
    {
        r = "  ";
    }
    if( p1 != -1 && p2 != -1 && p1 != p2 ) //ouverture et fermeture
    {
        r = "   ";
    }
    return r;
}
//-----------------------------------------------------------------------------
/*Retourne le NoteLocator suivant iC. Quand on atteint la fin de la
 barre, on retourne la note -1 de la prochaine barre. Il est important
 de retourner -1 pour que leffacement des barre et ligne se fasse
 correctement. voir doCommandErase.
 Quand il n'y a plus de note après iC, on retour iC*/
NoteLocator PartitionViewer::getNext( const NoteLocator& iC ) const
{
    int bar = iC.getBar(), index = iC.getIndex();
    if( iC.getIndex() == x->getNumberOfNotesInBar( iC.getBar() ) - 1 )
    {
        if( iC.getBar() + 1 < x->getNumberOfBars() )
        {
            bar = iC.getBar() + 1;
            index = -1;
        }
    }
    else{ index = iC.getIndex() + 1; }
    return NoteLocator( bar, index );
}
//-----------------------------------------------------------------------------
NoteLocator PartitionViewer::getNoteLocatorAtPosition(QPoint iP) const
{
    int barIndex = -1, tempBarIndex = -1;
    int noteIndex = -1;
    //Trouvons dans quelles barre est le point iP
    for(int i = 0; i < x->getNumberOfBars() && tempBarIndex == -1; ++i)
    {
        if(mBars[i].mScreenLayout.contains(iP))
        { tempBarIndex = i; }
    }
    
    if( tempBarIndex != -1 )
    {
        const Bar& b = getBar(tempBarIndex);
        //intersection avec les notes de la barres, on met le
        //curseur sur la note la plus proches. Si aucune notes
        //cliquée, on met au début de la barre.
        //intersection avec les mots de la barre
        bool onWord = false;
        for(int j = 0; j < (int)b.mWordScreenLayouts.size(); ++j )
        {
            if( b.mWordScreenLayouts[j].contains(iP) )
            {
                onWord = true;
                break;
            }
        }
        
        if(onWord)
        {
            double shortestDistance = numeric_limits<double>::max();
            for(size_t j = 0; j < b.mNoteScreenLayouts.size(); ++j )
            {
                QRectF r = b.mNoteScreenLayouts[j];
                QPointF p(iP.x(), iP.y());
                QPointF sub = r.center() - p;
                double d = sub.x()*sub.x() + sub.y()*sub.y();
                if( d < shortestDistance )
                {
                    shortestDistance = d;
                    barIndex = tempBarIndex;
                    noteIndex = j;
                }
            }
            
            //si on click a gauche du centre, on prend la note précédente,
            //sinon le note la plus pres du click.
            if( (iP.x() -
                 b.mNoteScreenLayouts[noteIndex].center().x()) < 0)
            { noteIndex = max(noteIndex - 1, -1); }
        }
    }
    
    return NoteLocator(barIndex, noteIndex);
}
//-----------------------------------------------------------------------------
int PartitionViewer::getNumberOfPages() const
{ return mNumberOfPages; }
//-----------------------------------------------------------------------------
int PartitionViewer::getNumberOfSelectedNote() const
{ return mSelectedNotes.size(); }
//-----------------------------------------------------------------------------
QRect PartitionViewer::getPageRegion( pageRegion iR, int iPage /*=0*/ ) const
{
    QRect r(0, 0, 1, 1);
    
    switch( iR )
    {
        case prPage:
        {
            int w = getPageSizeInInch().width() * logicalDpiX();
            int h = getPageSizeInInch().height() * logicalDpiY();
            int t = iPage * h + iPage * kInterPageGap;
            r.setTop( t );
            r.setWidth( w ); r.setHeight( h );
        }break;
        case prBody:
        {
            int margin = cmToPixel( kPageMarginInCm );
            int sargamScaleBottom = iPage == 0 ? getRegion(rAddRemoveDescriptionBar).bottom() + 2*kSpacing : 0;
            int startOfBody = max( margin, sargamScaleBottom );
            int paperWidth = getPageSizeInInch().width() * logicalDpiX();
            int paperHeight = getPageSizeInInch().height() * logicalDpiY();
            int topOfPage = getPageRegion( prPage, iPage ).top();//iPage * paperHeight + iPage * kInterPageGap;
            r.setLeft( margin );
            r.setWidth( paperWidth - 2*margin );
            r.setTop( topOfPage + startOfBody );
            r.setBottom( topOfPage + paperHeight - margin - kPageFooter );
        }break;
        case prPageFooter:
        {
            int margin = cmToPixel( kPageMarginInCm );
            r = getPageRegion( prPage, iPage );
            r.setBottom( r.bottom() - margin );
            r.setTop( r.bottom() - kPageFooter );
        }break;
    }
    
    return r;
}
//-----------------------------------------------------------------------------
QPageSize::PageSizeId PartitionViewer::getPageSizeId() const
{ return mPageSizeId; }
//-----------------------------------------------------------------------------
QSizeF PartitionViewer::getPageSizeInInch() const
{
    QSizeF s = QPageSize::size( mPageSizeId, QPageSize::Inch);
    
    if( getLayoutOrientation() == QPageLayout::Landscape )
    { s = QSizeF( s.height(), s.width() ); }
    
    return s;
}
//-----------------------------------------------------------------------------
NoteLocator PartitionViewer::getPrevious( const NoteLocator& iC ) const
{
    int bar = iC.getBar(), index = iC.getIndex();
    if (iC.getIndex() >= 0)
    { index = iC.getIndex() - 1; }
    else
    {
        if( iC.getBar() - 1 >= 0 )
        {
            bar = iC.getBar() - 1;
            index = x->getNumberOfNotesInBar( bar ) - 1;
        }
    }
    return NoteLocator( bar, index );
}
//-----------------------------------------------------------------------------
QRect PartitionViewer::getRegion( region iR ) const
{
    QRect r(0, 0, 1, 1);
    
    switch( iR )
    {
        case rPartition:
        {
            int w = getPageSizeInInch().width() * logicalDpiX();
            int h = getNumberOfPages() * getPageSizeInInch().height() * logicalDpiY() +
            std::max(getNumberOfPages() - 1, 0) * kInterPageGap;
            r.setWidth( w ); r.setHeight( h );
        }break;
        case rTitle:
        {
            int w = getPageSizeInInch().width() * logicalDpiX();
            int h = QFontMetrics(mTitleFont).height();
            r.setTop( cmToPixel( kPageMarginInCm ) ); r.setWidth( w ); r.setHeight( h );
        }break;
        case rDescriptionBars:
        {
            QRect t = getRegion( rTitle );
            r = t;
            r.translate( 0, t.height() + kSpacing );
            int margin = cmToPixel( kPageMarginInCm );
            r.setLeft( margin );
            r.setHeight( x->getNumberOfDescriptionBars() * getBarHeight(Bar::btDescription) );
        }break;
        case rAddRemoveDescriptionBar:
        {
            //on rajoute l'espace d'une barre de description pour faire place
            //aux boutons
            QRect t = getRegion( rDescriptionBars );
            r = t;
            r.translate( 0, t.height() );
            int margin = cmToPixel( kPageMarginInCm );
            r.setLeft( margin );
            r.setHeight( getBarHeight(Bar::btDescription) );
        }break;
    }
    return r;
}
//-----------------------------------------------------------------------------
QRect PartitionViewer::getRegion( descriptionBarRegion iR, int iIndex ) const
{
    QRect r(0, 0, 1, 1);
    
    switch( iR )
    {
        case dbrLabel:
        {
            r = getRegion( rDescriptionBars );
            r.setTop( r.top() +
                     r.height() / x->getNumberOfDescriptionBars() * iIndex );
            r.setHeight( getBarHeight( Bar::btDescription ) );
            
            QFontMetrics fm(mBarFont);
            r.setWidth( fm.width( x->getLabelFromDescriptionBar(iIndex) ) );
        }break;
        case dbrBar:
        {
            r = getRegion( rDescriptionBars );
            r.setTop( r.top() +
                     r.height() / x->getNumberOfDescriptionBars() * iIndex );
            r.setHeight( getBarHeight( Bar::btDescription ) );
            
            r.setLeft( getRegion( dbrLabel, iIndex ).right() + 5 );
        }break;
    }
    return r;
}
//-----------------------------------------------------------------------------
QString PartitionViewer::getParenthesisText( int iRep ) const
{
    QString s = QString::number( x->getNumberOfRepetitionsForParenthesis(iRep) );
    s += "x";
    return s;
}
//-----------------------------------------------------------------------------
int PartitionViewer::getParenthesisTextWidth( int iRep ) const
{
    QFontMetrics fm( mParenthesisFont );
    return fm.width( getParenthesisText( iRep ) );
}
//-----------------------------------------------------------------------------
NoteLocator PartitionViewer::getSelectedNote( int i ) const
{ return NoteLocator( mSelectedNotes[i].first, mSelectedNotes[i].second ); }
//-----------------------------------------------------------------------------
script PartitionViewer::getScript() const
{ return mScript; }
//-----------------------------------------------------------------------------
bool PartitionViewer::hasSelection() const
{ return mSelectedNotes.size() > 0; }
//-----------------------------------------------------------------------------
bool PartitionViewer::hasModifications() const
{ return mHasModifications; }
//-----------------------------------------------------------------------------
bool PartitionViewer::isDebugging() const
{ return mDebugMode != dmNone; }
//-----------------------------------------------------------------------------
bool PartitionViewer::isNoteSelected(int iBar, int iNoteIndex ) const
{
    bool r = false;
    for( int i = 0; i < mSelectedNotes.size(); ++i )
    {
        if( iBar == mSelectedNotes[i].first && iNoteIndex == mSelectedNotes[i].second )
        { r = true; break; }
    }
    return r;
}
//-----------------------------------------------------------------------------
bool PartitionViewer::hasBarTextEditionPending() const
{ return mEditingBarText != -1; }
//-----------------------------------------------------------------------------
bool PartitionViewer::hasDescriptionBarLabelEditionPending() const
{ return mEditingDescriptionBarLabel != -1; }
//-----------------------------------------------------------------------------
bool PartitionViewer::hasLineEditionPending() const
{ return mEditingLineIndex != -1; }
//-----------------------------------------------------------------------------
bool PartitionViewer::hasParenthesisEditionPending() const
{ return mEditingParentheseIndex != -1; }
//-----------------------------------------------------------------------------
void PartitionViewer::keyPressEvent( QKeyEvent* ipE )
{
    if( isVerbose() )
    { getLog().log( "PartitionViewer: key %s pressed.", QKeySequence( ipE->key(),
                                                                     QKeySequence::NativeText ).toString().toStdString().c_str() ); }
    
    switch ( ipE->key() )
    {
        case Qt::Key_1: commandAddNote( nvSa ); break;
        case Qt::Key_2: commandAddNote( nvRe ); break;
        case Qt::Key_3: commandAddNote( nvGa ); break;
        case Qt::Key_4: commandAddNote( nvMa );  break;
        case Qt::Key_5: commandAddNote( nvPa );  break;
        case Qt::Key_6: commandAddNote( nvDha );  break;
        case Qt::Key_7: commandAddNote( nvNi );  break;
        case Qt::Key_8: commandAddNote( Note(nvSa, 1) );  break;
        case Qt::Key_9: commandAddNote( Note(nvRe, 1) );  break;
        case Qt::Key_0: commandAddNote( Note(nvGa, 1) );  break;
        case Qt::Key_Comma: commandAddNote( nvComma ); break;
        case Qt::Key_C: commandAddNote( nvChik ); break;
        case Qt::Key_B: //Bend - meend
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandBreakOrnementsFromSelection(); }
            else{ commandAddOrnement( otMeend ); }
            break;
        case Qt::Key_A: //Andolan
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandBreakOrnementsFromSelection(); }
            else{ commandAddOrnement( otAndolan ); }
            break;
        case Qt::Key_E:
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandRemoveStroke(); }
            else{ commandAddStroke( stDiri );}
            break;
        case Qt::Key_G: //grace note
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandRemoveSelectionFromGraceNotes(); }
            else { commandAddGraceNotes(); }
            break;
        case Qt::Key_K: //krintan
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandBreakOrnementsFromSelection(); }
            else{ commandAddOrnement( otKrintan ); }
            break;
        case Qt::Key_M: //matra
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandBreakMatrasFromSelection(); }
            else{ commandAddMatra(); }
            break;
        case Qt::Key_N: //gamak
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandBreakOrnementsFromSelection(); }
            else{ commandAddOrnement( otGamak ); }
            break;
        case Qt::Key_P:
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandRemoveParenthesis(); }
            else { commandAddParenthesis(2); }
            break;
        case Qt::Key_Q:
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandRemoveStroke(); }
            else{ commandAddStroke( stDa ); }
            break;
        case Qt::Key_R: commandAddNote( nvRest );break;
        case Qt::Key_S: commandShiftNote();break;
        case Qt::Key_W:
            if( (ipE->modifiers() & Qt::ShiftModifier) )
            { commandRemoveStroke(); }
            else{ commandAddStroke( stRa );}
            break;
        case Qt::Key_Left:
        {
            /* 1. S'il n'y a pas de selection:
             Shift est enfoncé, donc on selectionne la note courante.
             Shift n'est pas enfoncé, donc on déplace la note courante
             2. S'il y a une selection:
             Shift est enfoncé, on agrandit la selection par la gauche.
             Shift n'est pas enfoncé, la selection disparait et la note courante
             devient la premiere note de la selection
             */
            bool shiftPressed = (ipE->modifiers() & Qt::ShiftModifier);
            if( !hasSelection() )
            {
                const NoteLocator nl(getCurrentBar(), getCurrentNote());
                const NoteLocator previous = getPrevious(nl);
                
                if( shiftPressed )
                { addNoteToSelection( nl.getBar(), nl.getIndex() ); }
                else
                { setCursorPosition(previous); }
            }
            else
            {
                int bar = mSelectedNotes[0].first;
                int index = mSelectedNotes[0].second;
                const NoteLocator nl(bar, index);
                
                if( shiftPressed )
                {
                    //ajoute la note a gauche de la premiere note selectionnee
                    const NoteLocator previous = getPrevious(nl);
                    addNoteToSelection( previous.getBar(), previous.getIndex() );
                }
                else
                {
                    setCursorPosition( getPrevious(nl) );
                    clearSelection();
                }
            }
        } break;
        case Qt::Key_Right:
        {
            /*
             1. S'il n'y a pas de selection:
             Shift est enfoncé, donc on selectionne la note courante.
             Shift n'est pas enfoncé, donc on déplace la note courante
             2. S'il y a une selection:
             Shift est enfoncé, on agrandit la selection par la gauche.
             Shift n'est pas enfoncé, la selection disparait et la note courante
             devient la premiere note de la selection */
            bool shiftPressed = (ipE->modifiers() & Qt::ShiftModifier);
            if( !hasSelection() )
            {
                const NoteLocator nextNl = getNext( NoteLocator(getCurrentBar(), getCurrentNote() ) );
                int bar = nextNl.getBar();
                int nextNote = nextNl.getIndex();
                
                if( shiftPressed )
                {
                    if( x->getNumberOfNotesInBar( bar ) > nextNote )
                    { addNoteToSelection( bar, nextNote ); }
                }
                else
                { setCursorPosition( NoteLocator(bar, nextNote) ); }
            }
            else
            {
                //on choisi la dernier note de la selection, pour étendre par
                //la droite
                int bar = mSelectedNotes[ mSelectedNotes.size() - 1 ].first;
                int index = mSelectedNotes[ mSelectedNotes.size() - 1 ].second;
                const NoteLocator nl(bar, index);
                const NoteLocator next = getNext(nl);
                if( shiftPressed )
                { addNoteToSelection( next.getBar(), next.getIndex() ); }
                else
                {
                    setCursorPosition( nl );
                    clearSelection();
                }
            }
        } break;
        case Qt::Key_Space: commandAddBar(); break;
        case Qt::Key_Backspace: commandErase(); break;
        case Qt::Key_Return: commandAddLine(); break;
        case Qt::Key_Plus: commandIncreaseOctave(); break;
        case Qt::Key_Minus: commandDecreaseOctave(); break;
        case Qt::Key_Shift: break;
        default: break;
    }
    updateUi();
    QPoint p( getCursorLine().p2().x(), getCursorLine().p2().y() );
    emit ensureVisible( p );
    emit interactionOccured();
}
//-----------------------------------------------------------------------------
void PartitionViewer::keyReleaseEvent( QKeyEvent* ipE )
{
    switch (ipE->key())
    {
        default: break;
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::mouseMoveEvent( QMouseEvent* ipE )
{
    QElapsedTimer _timer;
    _timer.start();
    
    QPoint pos = ipE->pos();
    Qt::CursorShape cs = Qt::ArrowCursor;
    
    //gestion du mouse state
    if(getMouseState() == msPressed)
    {
        const QPoint p = ipE->pos() - mMouseSelection.mPixelWhenPressed;
        if (p.manhattanLength() > 3)
        { setMouseState(msDraging); }
    }
    
    //intersection avec le titre
    if( mTitleScreenLayout.contains( pos ) )
    { cs = Qt::IBeamCursor; }
    
    //intersection avec les labels de barres de description
    for(int i = 0; i < x->getNumberOfDescriptionBars(); ++i)
    {
        if( getRegion(dbrLabel, i ).contains(pos) )
        {
            mDescriptionBarLabelHoverIndex = i;
            cs = Qt::IBeamCursor;
        }
        else if( mDescriptionBarLabelHoverIndex == i )
        { mDescriptionBarLabelHoverIndex = kNoBarIndex; }
    }
    
    //intersection avec le text de la barre courante
    const Bar& b = getBar( getCurrentBar() );
    if( b.mTextScreenLayout.contains( pos ) )
    {
        if( x->hasBarText( getCurrentBar() ) )
        { cs = Qt::IBeamCursor; }
    }
    
    //intersection avec les barres
    for( int i = 0; i < x->getNumberOfBars(); ++i )
    {
        const Bar& b = getBar(i);
        if( b.mScreenLayout.contains( pos ) )
        {
            mBarHoverIndex = i;
            
            //intersection avec les mots de la barre
            for(int j = 0; j < (int)b.mWordScreenLayouts.size(); ++j )
            {
                if( b.mWordScreenLayouts[j].contains(pos) )
                { cs = Qt::IBeamCursor; }
            }
        }
        else if ( mBarHoverIndex == i )
        {
            mBarHoverIndex = kNoBarIndex;
        }
    }
    
    //gestion de la selection par la souris
    if( getMouseState() == msDraging )
    {
        const NoteLocator nl = getNoteLocatorAtPosition(ipE->pos());
        if( nl.isValid() && mMouseSelection.mStart.isValid() )
        {
            mMouseSelection.mEnd = nl;
            clearSelection();
            
            //on selectionne vers la droite
            if(mMouseSelection.mStart < mMouseSelection.mEnd)
            {
                NoteLocator next = getNext(mMouseSelection.mStart);
                while (next < mMouseSelection.mEnd)
                {
                    mSelectedNotes.push_back( make_pair( next.getBar(), next.getIndex() ) );
                    next = getNext(next);
                }
                mSelectedNotes.push_back( make_pair( mMouseSelection.mEnd.getBar(), mMouseSelection.mEnd.getIndex() ) );
            }
            //on selectionne vers la gauche
            else if(mMouseSelection.mStart > mMouseSelection.mEnd)
            {
                mSelectedNotes.push_back( make_pair( mMouseSelection.mStart.getBar(), mMouseSelection.mStart.getIndex() ) );
                
                NoteLocator previous = getPrevious(mMouseSelection.mStart);
                while (previous > mMouseSelection.mEnd )
                {
                    mSelectedNotes.push_back( make_pair( previous.getBar(), previous.getIndex() ) );
                    previous = getPrevious(previous);
                }
                mSelectedNotes.push_back( make_pair( mMouseSelection.mEnd.getBar(), mMouseSelection.mEnd.getIndex() ) );
                
                //les notes doivent etre en ordre croissant pour
                //que les fonctions de navigation (voir keyPressed)
                //fonctionne correctement.
                sort( mSelectedNotes.begin(), mSelectedNotes.end() );
            }
            else {}
            updateUi();
        }
        
    }
    
    //intersection avec le texte des lignes
    for( int i = 0; i < x->getNumberOfLines(); ++i )
    {
        if( mLines[i].mTextScreenLayout.contains( pos ) )
        { cs = Qt::IBeamCursor; }
    }
    
    //intersection avec le texte des parentheses
    for( int i = 0; i < x->getNumberOfParenthesis(); ++i )
    {
        if( mParenthesis[i].mTextScreenLayout.contains(pos) )
        { cs = Qt::IBeamCursor; }
    }
    
    setCursor( cs );
    
    if( hasLogTiming() )
    {
        getLog().log("PartitionViewer::mouseMove: %.3f ms", _timer.nsecsElapsed() / 1000000.0 );
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::mousePressEvent(QMouseEvent* ipE)
{
    setMouseState(msPressed);
    mMouseSelection.mPixelWhenPressed = ipE->pos();
    
    //selection de la barre et de la note
    if( mBarHoverIndex != kNoBarIndex )
    {
        clearSelection();
        
        NoteLocator nl = getNoteLocatorAtPosition(ipE->pos());
        if(nl.isValid())
        { mMouseSelection.mStart = nl; }
        else
        {
            mMouseSelection.mStart = NoteLocator::invalid();
            nl = NoteLocator(mBarHoverIndex, -1);
        }
        setCursorPosition(nl);
        
        //animation du highlight de la barre courante
        if(mCurrentBarTimerId != 0) { killTimer(mCurrentBarTimerId); }
        mCurrentBarTimerId = startTimer(60);
        mCurrentBarTimer.start();
        
        updateUi();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::mouseReleaseEvent( QMouseEvent* ipE )
{
    QPoint pos = ipE->pos();
    
    //intersection avec le titre
    if( mTitleScreenLayout.contains( pos ) )
    { startTitleEdit(); }
    
    //intersection avec les labels des barres de description
    if(mDescriptionBarLabelHoverIndex != kNoBarIndex)
    {
        startDescriptionBarLabelEdit( mDescriptionBarLabelHoverIndex );
        mDescriptionBarLabelHoverIndex = -1;
    }
    
    //intersection avec le text de la barre courante
    const Bar& currentB = getBar( getCurrentBar() );
    if( currentB.mTextScreenLayout.contains( pos ) )
    {
        startBarTextEdit( getCurrentBar() );
    }
    
    //intersection avec le texte des lignes
    for( int i = 0; i < x->getNumberOfLines(); ++i )
    {
        if( mLines[i].mTextScreenLayout.contains( pos ) )
        { startLineTextEdit( i ); }
    }
    
    //intersection avec le texte des parentheses
    for( int i = 0; i < x->getNumberOfParenthesis(); ++i )
    {
        if( mParenthesis[i].mTextScreenLayout.contains(pos) )
        { startParentheseEdit(i); }
    }
    
    //gestion de la selection par dragging;
    setMouseState(msIdle);
    
    emit interactionOccured();
}
//-----------------------------------------------------------------------------
void PartitionViewer::moveGraceNoteBackward(int iFromBar, int iToIndex)
{
    for( int i = 0; i < x->getNumberOfGraceNotesInBar(iFromBar); ++i )
    {
        int ni = x->getNoteIndexFromGraceNote(iFromBar, i) + iToIndex;
        { addNoteToSelection(iFromBar-1, ni); }
        doCommandAddGraceNotes();
        clearSelection();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::moveGraceNoteForward(int iFromBar, int iFromIndex)
{
    for( int i = 0; i < x->getNumberOfGraceNotesInBar(iFromBar); ++i )
    {
        int ni = x->getNoteIndexFromGraceNote(iFromBar, i) - iFromIndex;
        if( ni >= 0 )
        { addNoteToSelection(iFromBar+1, ni); }
        doCommandAddGraceNotes();
        clearSelection();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::moveMatraBackward(int iFromBar, int iToIndex)
{
    for( int i = 0; i < x->getNumberOfMatraInBar(iFromBar); ++i )
    {
        for( int j = 0; j < x->getNumberOfNotesInMatra(iFromBar, i); ++j )
        {
            int ni = x->getNoteIndexFromMatra(iFromBar, i, j) + iToIndex;
            { addNoteToSelection(iFromBar - 1, ni ); }
        }
        doCommandAddMatra();
        clearSelection();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::moveMatraForward(int iFromBar, int iFromIndex)
{
    for( int i = 0; i < x->getNumberOfMatraInBar(iFromBar); ++i )
    {
        for( int j = 0; j < x->getNumberOfNotesInMatra(iFromBar, i); ++j )
        {
            int ni = x->getNoteIndexFromMatra(iFromBar, i, j) - iFromIndex;
            if( ni >= 0 )
            { addNoteToSelection(iFromBar+1, ni ); }
        }
        doCommandAddMatra();
        clearSelection();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::moveOrnementBackward(int iFromBar, int iToIndex)
{
    /*ramasse toutes les notes de la bar iFromBar*/
    vector<NoteLocator> vn;
    for( int i = 0; i < x->getNumberOfNotesInBar(iFromBar); ++i )
    { vn.push_back( NoteLocator( iFromBar, i) ); }
    
    //ramasse tous les ornements à déplacer
    set<int> ornToMove;
    for( int i = 0; i < vn.size(); ++i )
    {
        int ornIndex = x->findOrnement( vn[i].getBar(), vn[i].getIndex() );
        if( ornIndex >= 0 ) { ornToMove.insert( ornIndex ); }
    }
    
    /*pour chaque ornement, si la barre du noteLocator est égale à iFromBar
     on décremente la barre puisqu'on veut déplacer la note dans la barre
     précédente et on ajoute ce NoteLocator à la selection. Il faut imaginer
     cette opération comme si on la faisait à bras. On commencerait par mettre
     les notes de la barre courante dans la barre précedente, ensuite on
     selectionnerait le note pour refaire le même ornement et ensuite on
     effacerait la barre voulue.*/
    set<int>::iterator it = ornToMove.begin();
    for( ; it != ornToMove.end(); ++it )
    {
        ornementType ot = x->getOrnementType( *it );
        for(int i = 0; i < x->getNumberOfNotesInOrnement( *it ); ++i )
        {
            NoteLocator nl = x->getNoteLocatorFromOrnement( *it, i);
            int bar = nl.getBar();
            int index = nl.getIndex();
            if( bar == iFromBar )
            { bar--; index = index + iToIndex; }
            addNoteToSelection(bar, index);
        }
        doCommandAddOrnement(ot);
        clearSelection();
    }
}
//-----------------------------------------------------------------------------
/*voir moveOrnementBackward*/
void PartitionViewer::moveOrnementForward(int iFromBar, int iFromIndex)
{
    vector<NoteLocator> vn;
    for( int i = iFromIndex; i < x->getNumberOfNotesInBar(iFromBar); ++i )
    { vn.push_back( NoteLocator( iFromBar, i) ); }
    
    set<int> ornToMove;
    for( int i = 0; i < vn.size(); ++i )
    {
        int ornIndex = x->findOrnement( vn[i].getBar(), vn[i].getIndex() );
        if( ornIndex >= 0 ) { ornToMove.insert( ornIndex ); }
    }
    set<int>::iterator it = ornToMove.begin();
    for( ; it != ornToMove.end(); ++it )
    {
        ornementType ot = x->getOrnementType( *it );
        for(int i = 0; i < x->getNumberOfNotesInOrnement( *it ); ++i )
        {
            NoteLocator nl = x->getNoteLocatorFromOrnement( *it, i);
            int bar = nl.getBar();
            int index = nl.getIndex();
            if( bar == iFromBar && index >= iFromIndex )
            {
                bar++;
                index -= iFromIndex;
            }
            addNoteToSelection(bar, index);
        }
        doCommandAddOrnement(ot);
        clearSelection();
    }
}
//-----------------------------------------------------------------------------
//voir moveOrnementBackward... c'est pareil
void PartitionViewer::moveParenthesisBackward( int iFromBar, int iToIndex )
{
    /*ramasse toutes les notes de la bar iFromBar*/
    vector<NoteLocator> vn;
    for( int i = 0; i < x->getNumberOfNotesInBar(iFromBar); ++i )
    { vn.push_back( NoteLocator( iFromBar, i) ); }
    
    //ramasse tous les ornements à déplacer
    set<int> pToMove;
    for( int i = 0; i < vn.size(); ++i )
    {
        int p = x->findParenthesis( vn[i].getBar(), vn[i].getIndex() );
        if( p >= 0 ) { pToMove.insert( p ); }
    }
    
    /*pour chaque parentheses, si la barre du noteLocator est égale à iFromBar
     on décremente la barre puisqu'on veut déplacer la note dans la barre
     précédente et on ajoute ce NoteLocator à la selection. Il faut imaginer
     cette opération comme si on la faisait à bras. On commencerait par mettre
     les notes de la barre courante dans la barre précedente, ensuite on
     selectionnerait le note pour refaire le même parenthese et ensuite on
     effacerait la barre voulue.*/
    set<int>::iterator it = pToMove.begin();
    for( ; it != pToMove.end(); ++it )
    {
        int numberOfTime = x->getNumberOfRepetitionsForParenthesis( *it );
        for(int i = 0; i < x->getNumberOfNotesInParenthesis( *it ); ++i )
        {
            NoteLocator nl = x->getNoteLocatorFromParenthesis( *it, i);
            int bar = nl.getBar();
            int index = nl.getIndex();
            if( bar == iFromBar )
            { bar--; index = index + iToIndex; }
            addNoteToSelection(bar, index);
        }
        doCommandAddParenthesis( numberOfTime );
        clearSelection();
    }
}
//-----------------------------------------------------------------------------
//voir moveParenthesisBackward
void PartitionViewer::moveParenthesisForward( int iFromBar, int iFromIndex )
{
    vector<NoteLocator> vn;
    for( int i = iFromIndex; i < x->getNumberOfNotesInBar(iFromBar); ++i )
    { vn.push_back( NoteLocator( iFromBar, i) ); }
    
    set<int> pToMove;
    for( int i = 0; i < vn.size(); ++i )
    {
        int index = x->findParenthesis( vn[i].getBar(), vn[i].getIndex() );
        if( index >= 0 ) { pToMove.insert( index ); }
    }
    set<int>::iterator it = pToMove.begin();
    for( ; it != pToMove.end(); ++it )
    {
        int numberOfTime = x->getNumberOfRepetitionsForParenthesis( *it );
        for(int i = 0; i < x->getNumberOfNotesInParenthesis( *it ); ++i )
        {
            NoteLocator nl = x->getNoteLocatorFromParenthesis( *it, i);
            int bar = nl.getBar();
            int index = nl.getIndex();
            if( bar == iFromBar && index >= iFromIndex )
            {
                bar++;
                index -= iFromIndex;
            }
            addNoteToSelection(bar, index);
        }
        doCommandAddParenthesis( numberOfTime );
        clearSelection();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::moveStrokeBackward(int iFromBar, int iToIndex)
{
    for( int i = 0; i < x->getNumberOfStrokesInBar(iFromBar); ++i )
    {
        strokeType st = x->getStrokeType(iFromBar, i);
        for( int j = 0; j < x->getNumberOfNotesInStroke(iFromBar, i); ++j )
        {
            int ni = x->getNoteIndexFromStroke(iFromBar, i, j) + iToIndex;
            { addNoteToSelection(iFromBar - 1, ni ); }
        }
        if( hasSelection() )
        {
            doCommandAddStroke(st);
            clearSelection();
        }
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::moveStrokeForward(int iFromBar, int iFromIndex)
{
    for( int i = 0; i < x->getNumberOfStrokesInBar(iFromBar); ++i )
    {
        strokeType st = x->getStrokeType(iFromBar, i);
        for( int j = 0; j < x->getNumberOfNotesInStroke(iFromBar, i); ++j )
        {
            int ni = x->getNoteIndexFromStroke(iFromBar, i, j) - iFromIndex;
            if( ni >= 0 )
            { addNoteToSelection(iFromBar+1, ni ); }
        }
        if( hasSelection() )
        {
            doCommandAddStroke(st);
            clearSelection();
        }
    }
}
//-----------------------------------------------------------------------------
QString PartitionViewer::noteToString( Note iNote ) const
{
    int note = iNote.getValue();
    noteModification nm = iNote.getModification();
    
    QString r;
    switch( getScript() )
    {
        default:
        case sLatin:
        {
            switch( note )
            {
                case nvSa: r = "S"; break;
                case nvRe: if(nm == nmKomal){r = "r";}else {r = "R";} break;
                case nvGa: if(nm == nmKomal){r = "g";}else {r = "G";} break;
                case nvMa: if(nm == nmTivra){r = "M";}else {r = "m";} break;
                case nvPa: r = "P"; break;
                case nvDha: if(nm == nmKomal){r = "d";}else {r = "D";} break;
                case nvNi: if(nm == nmKomal){r = "n";}else {r = "N";} break;
                case nvChik: r = "\xE2\x9c\x93"; break; //check
                case nvRest: r = "\xE2\x80\x94"; break; //barre horizontale
                case nvComma: r = ","; break;
                default: break;
            }
        }  break;
        case sDevanagari:
        {
            switch( note )
            {
                case nvSa: r = ("\xE0\xA4\xB8"); /*स*/ break;
                case nvRe:
                    r = QString::fromUtf8("\xE0\xA4\xB0") + QString::fromUtf8("\xE0\xA5\x93");
                    break;
                case nvGa: r = ("\xE0\xA4\x97"); break;
                case nvMa: r = (nm == nmTivra) ?  QString::fromUtf8("\xE0\xA4\xAE") + QString::fromUtf8("\xE0\xA5\x91") : "\xE0\xA4\xAE"; break; //Ma + Stress Sign Udatta
                case nvPa: r = ("\xE0\xA4\xAA"); break;
                case nvDha: r =("\xE0\xA4\xA7"); break;
                case nvNi: r = QString::fromUtf8("\xE0\xA4\xA8") + QString::fromUtf8("\xE0\xA5\x80"); break; //na + vowel sign Ii
                case nvChik: r = ("\xE2\x9c\x93"); break; //check
                case nvRest: r = ("\xE2\x80\x94"); break; //barre horizontale
                case nvComma: r = ","; break;
                default: break;
            }
        }break;
    }
    
    return r;
}
//-----------------------------------------------------------------------------
void PartitionViewer::paintEvent( QPaintEvent* ipE )
{
    draw( this, ipE->region().boundingRect() );
    
    //--- render debug infos
    if( isDebugging() )
    {
        QPainter p(this);
        p.setFont(QFont("Arial", 10));
        p.setBackgroundMode( Qt::OpaqueMode );
        p.setBrush( Qt::NoBrush );
        
        p.setPen( Qt::blue );
        p.drawRect( mTitleScreenLayout );
        
        p.setPen( Qt::yellow );
        for( int i = 0; i < getNumberOfPages(); ++i)
        {
            if( i == 0 )
            {
                p.drawRect( getRegion( rTitle ) );
                p.drawRect( getRegion(rDescriptionBars) );
                p.drawRect( getRegion(rAddRemoveDescriptionBar) );
            }
            p.drawRect( getPageRegion( prBody, i ) );
            p.drawRect( getPageRegion( prPageFooter, i ) );
        }
        
        //lemplace du bouton add description bar
        QPen pen = p.pen();
        pen.setColor(Qt::black);
        pen.setWidth(3);
        p.setPen(pen);
        p.drawPoint(mAddDescriptionBarButtonPos);
        
        //on dessine le layout des bars...
        for( int i = 0; i < x->getNumberOfBars(); ++i )
        {
            p.setPen( Qt::green );
            Bar& b = getBar(i);
            //le layput page de la bar
            p.drawRect( b.mScreenLayout );
            
            if(i < x->getNumberOfDescriptionBars())
            { p.drawRect( getRegion(dbrLabel, i) ); }
            
            //layout du text
            p.drawRect( b.mTextScreenLayout );
            
            //layout de mots
            if( getDebugMode() == dmWordLayout )
            {
                p.setPen( Qt::blue );
                for( int j = 0; j < b.mWordScreenLayouts.size(); ++j )
                { p.drawRect( b.mWordScreenLayouts[j] ); }
            }
            
            //le layout page des notes
            if( getDebugMode() == dmNoteLayout )
            {
                p.setPen( Qt::green );
                for( int j = 0; j < b.mNoteScreenLayouts.size(); ++j )
                { p.drawRect( b.mNoteScreenLayouts[j] ); }
            }
            
            //texte de debuggage
            if( getDebugMode() == dmBarInfo )
            {
                p.setPen(Qt::gray);
                p.drawText( b.mScreenLayout, Qt::AlignCenter, QString::number(i) );
                QString s;
                if( getCurrentBar() == i )
                { s += QString().sprintf("current note: %d\n", getCurrentNote() ); }
                s += QString().sprintf("number of notes: %d", x->getNumberOfNotesInBar( i ) );
                p.drawText( b.mScreenLayout, Qt::AlignBottom | Qt::AlignRight, s );
            }
        }
        
        p.setPen( Qt::blue );
        //layout des lignes
        for( int i = 0; i < x->getNumberOfLines(); ++i )
        {
            const Line& l = mLines[i];
            p.drawRect( l.mLineNumberRect );
            p.drawRect( l.mHotSpot );
            p.drawRect( l.mTextScreenLayout );
        }
        
        //layout des Parenthesis
        for( int i = 0; i < x->getNumberOfParenthesis(); ++i )
        {
            const Parenthesis& r = mParenthesis[i];
            p.drawRect( r.mOpeningScreenLayout );
            p.drawRect( r.mClosingScreenLayout );
            p.drawRect( r.mTextScreenLayout );
        }
        
        //--- different texte...
        p.setPen(Qt::gray);
        //posiition de la souris, dans le coin sup gauche...
        {
            p.save();
            QBrush b( Qt::white );
            p.setBackgroundMode( Qt::OpaqueMode );
            p.setBackground( b );
            QRect r = ipE->region().boundingRect();
            QPoint pos = mapFromGlobal( QCursor::pos() );
            QString s;
            s.sprintf( "Mouse pos: %d, %d\n"
                      "Region to repaint: %d, %d, %d, %d\n"
                      "Number of Ornement: %d\n"
                      "Number of parenthesis: %d\n",
                      pos.x(), pos.y(),
                      r.left(), r.top(), r.right(), r.bottom(),
                      x->getNumberOfOrnements(),
                      x->getNumberOfParenthesis() ) ;
            p.drawText( r.topLeft() + QPoint(10, 10), s );
            p.restore();
        }
        
        QString s;
        s.sprintf("Partition size: %d, %d\n",
                  getRegion( rPartition ).width(),
                  getRegion( rPartition ).height() );
        p.drawText(rect(), Qt::AlignBottom | Qt::AlignRight, s);
        
        /*print des barres par pages...*/
        //    for( int i = 0; i < getNumberOfPages(); ++i )
        //    {
        //      vector<int> bars = getBarsFromPage( i );
        //      for( int j = 0; j < bars.size(); ++j )
        //      { printf( "page %d, bar %d\n", i, bars[j] ); }
        //    }
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::print( QPrinter* iPrinter )
{
    QPageSize::PageSizeId previousPageSizeId = getPageSizeId();
    QPageLayout::Orientation previousOrientation = getLayoutOrientation();
    
    QPageLayout pl = iPrinter->pageLayout();
    
    setPageSize( pl.pageSize().id() );
    setLayoutOrientation( pl.orientation() );
    
    QPainter p( iPrinter );
    
    //--- titre
    drawTitle( &p );
    
    //--- rendu des textes pour barres description
    drawDescriptionBars( &p );
    
    for( int i = 0; i < getNumberOfPages(); ++i )
    {
        p.save();
        QTransform pageTransfo;
        QPoint pageTop = getPageRegion( prPage, i ).topLeft();
        pageTop.setY( pageTop.y() - i * kInterPageGap );
        
        pageTransfo.translate( -pageTop.x(), -pageTop.y() );
        p.setWorldTransform( pageTransfo );
        
        vector<int> bars = getBarsFromPage( i );
        for( int j = 0; j < bars.size(); ++j )
        {
            drawBar( &p, bars[j] );
            if( x->isStartOfLine( bars[j] ) )
            { drawLine( &p, x->findLine( bars[j] ) ); }
        }
        
        drawPageFooter( &p, i );
        
        p.restore();
        if( i < getNumberOfPages() - 1 )
        { iPrinter->newPage(); }
    }
    
    //on remet la taille du papier...
    setPageSize( previousPageSizeId );
    setLayoutOrientation( previousOrientation );
}
//-----------------------------------------------------------------------------
void PartitionViewer::redoActivated()
{ mUndoRedoStack.redo(); }
//-----------------------------------------------------------------------------
void PartitionViewer::removeDescriptionBarClicked()
{
    CommandRemoveDescriptionBar* c = new CommandRemoveDescriptionBar(this);
    c->execute();
    mUndoRedoStack.add(c);
}
//-----------------------------------------------------------------------------
// clears all ui cache regarding composition and sets internal pointer
// see setComposition to start with a clean slate (like opening a new
// file). This method is mostly used by command for undoing changes.
void PartitionViewer::resetComposition(Composition* ipC)
{
    clear();
    x = ipC != 0 ? ipC : &mDummyComposition;
    mBars.resize( ipC->getNumberOfBars() );
    
    //--- lines
    mLines.resize( ipC->getNumberOfLines() );
    
    //--- ornements
    mOrnements.resize( ipC->getNumberOfOrnements() );
    
    setCursorPosition(NoteLocator(0, -1));
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::resizeEditToContent()
{
    QLineEdit* le = dynamic_cast<QLineEdit*>( QObject::sender() );
    QSpinBox* sb = dynamic_cast<QSpinBox*>( QObject::sender() );
    if( le )
    { resizeLineEditToContent(le); }
    else if(sb)
    { resizeSpinBoxToContent(sb); }
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::resizeLineEditToContent(QLineEdit* ipLe)
{
    QFontMetrics fm( ipLe->font() );
    ipLe->resize( std::max( fm.width( ipLe->text() + "a" ),
                           fm.width( "short" ) ), fm.height() );
}
//-----------------------------------------------------------------------------
void PartitionViewer::resizeSpinBoxToContent(QSpinBox* ipSb)
{
    QFontMetrics fm( ipSb->font() );
    ipSb->resize( (double)fm.width( "99" ) + 30, ipSb->height() );
}
//-----------------------------------------------------------------------------
void PartitionViewer::setAllBarsAsDirty( bool iD )
{
    for( int i = 0; i < x->getNumberOfBars(); ++i )
    { setBarAsDirty(i, iD); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::setAsVerbose( bool iV )
{ mIsVerbose = iV; }
//-----------------------------------------------------------------------------
void PartitionViewer::setBarAsDirty( int iBar, bool iD )
{ getBar(iBar).mIsDirty = iD; }
//-----------------------------------------------------------------------------
void PartitionViewer::setBarsAsDirty( vector<int> iV, bool iD )
{
    for( int i = 0; i < iV.size(); ++i )
    { setBarAsDirty( iV[i], iD ); }
}
//-----------------------------------------------------------------------------
// Clear all ui cache, undo/redo stack and sets the composition to ipC
void PartitionViewer::setComposition(Composition* ipC)
{
    mUndoRedoStack.clear();
    setAsModified(false);
    
    resetComposition(ipC);
}
//-----------------------------------------------------------------------------
//sets the current bar. See also setCursorPosition
void PartitionViewer::setCurrentBar( int iB )
{
    mCurrentBar = iB;
    updateUi();
}
//-----------------------------------------------------------------------------
//La note courante ne doit pas etre inférieur a -1. La valeur -1 indique,
//que le curseur est au début de la barre, juste avant la note 0.
//voir aussi setCursorPosition
void PartitionViewer::setCurrentNote( int iN )
{ mCurrentNote = max( iN, -1 ); updateUi(); }
//-----------------------------------------------------------------------------
void PartitionViewer::setCursorPosition( NoteLocator iNl )
{
    setCurrentBar(iNl.getBar());
    setCurrentNote(iNl.getIndex());
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::setFontSize(int iFontSize)
{
    int delta = iFontSize - mBarFont.pointSize();
    
    mTitleFont.setPointSize( mTitleFont.pointSize() + delta );
    mBarFont.setPointSize( mBarFont.pointSize() + delta );
    mBarTextFont.setPointSize( mBarTextFont.pointSize() + delta );
    mGraceNotesFont.setPointSize( mGraceNotesFont.pointSize() + delta );
    mLineFont.setPointSize( mLineFont.pointSize() + delta );
    mStrokeFont.setPointSize( mStrokeFont.pointSize() + delta );
    mParenthesisFont.setPointSize( mParenthesisFont.pointSize() + delta );
    
    mBaseParenthesisRect = QRectF( QPoint(0, 0),
                                  QSize(8, QFontMetrics(mBarFont).height() * 1.8) );
    
    //on met toutes les barres dirty
    setAllBarsAsDirty(true);
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::setLayoutOrientation( QPageLayout::Orientation iO )
{
    mLayoutOrientation = iO;
    updateLayout();
    QRect p = getRegion( rPartition );
    resize( p.width()+1, p.height()+1 );
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::setLog( Log* ipLog )
{
    if( ipLog )
    {
        ipLog->takeEntriesFrom( *mpLog );
        mpLog = ipLog;
    }
    else{ ipLog = &mDefaultLog; }
}
//-----------------------------------------------------------------------------
void PartitionViewer::setPageSize( QPageSize::PageSizeId iId )
{
    mPageSizeId = iId;
    updateLayout();
    QRect p = getRegion( rPartition );
    resize( p.width()+1, p.height()+1 );
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::setNumberOfPage( int iN )
{ mNumberOfPages = iN; }
//-----------------------------------------------------------------------------
void PartitionViewer::setScript( script iS )
{
    if( mScript != iS)
    {
        mScript = iS;
        setAllBarsAsDirty(true);
    }
    updateUi();
}
//-----------------------------------------------------------------------------
//cette méthode sert a garantir que tous les éléments du layout qui
//dépendent de widgets sont correctement calculé lorsque l'application
//apparait a l'écran la première fois.
void PartitionViewer::showEvent(QShowEvent* ipSe)
{
    updateLayout();
    updateUi();
}
//-----------------------------------------------------------------------------
/*Permet de separer en vecteur de note index par barre une selection iV qui
 contient des notes sur plusieurs barres.*/
map< int, vector<int> > PartitionViewer::splitPerBar(
                                                     vector< std::pair<int, int> > iV ) const
{
    map< int, vector<int> > r;
    
    int bar = -1, noteIndex = -1;
    vector< int > bg;
    for( int i = 0; i < iV.size(); ++i )
    {
        if( bar != iV[i].first && !bg.empty() )
        {
            r[bar] = bg;
            bg.clear();
        }
        bar = iV[i].first;
        noteIndex = iV[i].second;
        bg.push_back( noteIndex );
    }
    r[bar] = bg;
    
    return r;
}
//-----------------------------------------------------------------------------
/*Cette fonction coupe les notes de la barre en mots. Pour chaque changement
 de fonte du texte, on crée un nouveau mot. Les mots seront ensuite rendu à
 l'écran. Lobjectif de séparer en mots est de pouvoir rendre correctement le
 texte multilangue à l'écran.
 De plus, pour chaque mot, c'est ici qu'est calculé le rectangle qui encapsule
 chaque note. Étant donnée le support devanagari, pour lequel une note represente
 plusieurs caracteres ascii, la tâche se complique. C'est ce qui motive
 l'utilisation de QTexLayout, QTextBoundaryFinder et QGlyphRun.
 */
void PartitionViewer::splitInWords(int iBar)
{
    Bar& b = getBar(iBar);
    b.mNotesRect.clear();
    b.mWords.clear();
    b.mWordLayouts.clear();
    
    bool isGraceNote = false;
    QString text;
    /*On ajoute chaque note dans text et lors d'un changement de fonte,
     on insere text dans le vecteur words et on commence un nouveau mot.*/
    for( int i = 0; i < x->getNumberOfNotesInBar( iBar ); ++i )
    {
        isGraceNote = x->isGraceNote(iBar, i);
        text += noteToString(x->getNote(iBar, i));
        if( (i+1) < x->getNumberOfNotesInBar(iBar) )
        {
            NoteLocator cnl(iBar, i);
            NoteLocator nnl = getNext(cnl);
            text += getInterNoteSpacingAsQString(cnl, nnl);
            //si la prochaine note demande un changement de font, on
            //coupe le string.
            if( isGraceNote != x->isGraceNote(iBar, i+1) )
            {
                b.mWords.push_back( make_pair(text, isGraceNote) );
                text = QString();
            }
        }
    }
    if( !text.isNull() )
    {
        /*on ajoute toujours un espace a la fin de la barre pour aérer.*/
        text += " ";
        b.mWords.push_back( make_pair(text, isGraceNote) );
    }
    
    /*Pour chaque mots, on fabrique le rectangle qui encapsulera ce mot. Ce
     rectangle est le resultat de la composition des rectangle des glyphs du mots.*/
    int cursorX = getBarRegion( brNoteStartX, b.mBarType );
    for(size_t i = 0; i < b.mWords.size(); ++i)
    {
        QString word = b.mWords[i].first;
        QFont f = b.mWords[i].second ? mGraceNotesFont : mBarFont;
        const int offsetX = cursorX;
        const int posY = b.mWords[i].second ? getBarRegion(brGraceNoteTopY, b.mBarType) :
        getBarRegion(brNoteTopY, b.mBarType);
        QRectF wordRect;
        
        //printf("word[%d] on bar %d: -%s-\n", (int)i, iBar, word.toStdString().c_str());
        
        QTextLayout tl( word );
        tl.setFont( f );
        tl.beginLayout();
        QTextLine textLine = tl.createLine();
        //on met 2 fois la taille de la page par mesure de pécaution.
        //Lorsqu'on atteint la taille de textLine, la methode
        //glyphRuns() retourne 0... ce qui nous cause des problèmes.
        //Étant donné, qu'on ne gère pas encore les barres plus grande
        //qu'une ligne, on ajoute cette mesure de précaution ici pour ne
        //pas crasher l'application dès que la barre atteint la taille de
        //la page.
        textLine.setLineWidth( 2*getPageRegion( prBody, 0 ).width() );
        tl.endLayout();
        
        //printf("textLayout on bar %d: -%s-\n", iBar, tl.text().toStdString().c_str());
        //    QList<QGlyphRun> grl = tl.glyphRuns(0, word.size());
        //{
        //printf("num glyph on line %d\n", grl.size());
        //for(int j = 0; j<grl.size(); ++j)
        //{
        //  QRectF r = grl[j].boundingRect();
        //  printf("\tglyph %d bounding rect: pointf(%.2f,%.2f) size(%.2f,%.2f)\n",
        //    j, r.topLeft().x(), r.topLeft().y(), r.width(), r.height());
        //}
        //}
        
        /*QtextBoundaryFinder sert a trouver les indices dans le string qui
         forme un grapheme (une note).*/
        QTextBoundaryFinder bf(QTextBoundaryFinder::Grapheme, word);
        bf.toStart();
        int glyphStartsAt = 0;
        int glyphEndsAt = bf.toNextBoundary();
        /*on va trouver le rectangle qui encapsule chaque note. Ce
         rectangle servira a placer le curseur, calculer la tailler des ornements et
         calculer les boites de selection. */
        while( glyphEndsAt != -1 && glyphStartsAt < word.length() )
        {
            //printf("%d, %d, %d, -%s-\n", glyphStartsAt, glyphEndsAt, glyphEndsAt - glyphStartsAt,
            //       word.mid(glyphStartsAt, glyphEndsAt - glyphStartsAt).toStdString().c_str());
            
            QList<QGlyphRun> grl = tl.glyphRuns(glyphStartsAt, glyphEndsAt - glyphStartsAt);
            assert( grl.size() == 1 );
            //printf("grl size: %d\n", (int)grl.size());
            
            QRectF glyphRect = grl[0].boundingRect();
            glyphRect = QRectF( QPointF(glyphRect.left() + offsetX, posY),
                               QSize(glyphRect.width(), glyphRect.height() ) );
            
            //on n'ajoute pas les espace...
            if( word.mid(glyphStartsAt, glyphEndsAt - glyphStartsAt) != " " )
            {b.mNotesRect.push_back(glyphRect);}
            
            wordRect = wordRect.united( glyphRect );
            
            glyphStartsAt = glyphEndsAt;
            glyphEndsAt = bf.toNextBoundary();
        }
        
        //on increment le curseur de la taille du mot.
        cursorX += wordRect.width();
        b.mWordLayouts.push_back(wordRect);
    }
}

//-----------------------------------------------------------------------------
void PartitionViewer::startBarTextEdit( int iBar )
{
    mpBarTextEdit->setFocus();
    mpBarTextEdit->setFont(mBarTextFont);
    mpBarTextEdit->setText( x->getBarText( iBar ) );
    resizeLineEditToContent(mpBarTextEdit);
    mEditingBarText = iBar;
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::startDescriptionBarLabelEdit( int iBar )
{
    mpDescriptionBarLabelEdit->setFocus();
    //  mpBarTextEdit->setFont(mBarFont);
    mpDescriptionBarLabelEdit->setText( x->getLabelFromDescriptionBar(iBar) );
    resizeLineEditToContent(mpDescriptionBarLabelEdit);
    mEditingDescriptionBarLabel = iBar;
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::startLineTextEdit( int iLineIndex )
{
    mpLineTextEdit->setFocus();
    mpLineTextEdit->setFont(mLineFont);
    mpLineTextEdit->setText( x->getLineText(iLineIndex) );
    resizeLineEditToContent(mpLineTextEdit);
    mEditingLineIndex = iLineIndex;
    updateLayout(); //pour afficher correctement le lineEdit
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::startParentheseEdit(int iIndex)
{
    mpParenthesisEdit->setFocus();
    mpParenthesisEdit->setFont(mParenthesisFont);
    mpParenthesisEdit->setValue(x->getNumberOfRepetitionsForParenthesis(iIndex));
    resizeSpinBoxToContent(mpParenthesisEdit);
    mEditingParentheseIndex = iIndex;
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::startTitleEdit()
{
    mpTitleEdit->setFocus();
    mpTitleEdit->setFont(mTitleFont);
    mpTitleEdit->setText( x->getTitle() );
    resizeLineEditToContent(mpTitleEdit);
    mEditingTitle = true;
    updateUi();
}
//-----------------------------------------------------------------------------
void PartitionViewer::stopBarTextEdit()
{
    if( hasBarTextEditionPending() )
    {
        x->setBarText( mEditingBarText, mpBarTextEdit->text() );
        setBarAsDirty( mEditingBarText, true );
        mEditingBarText = -1;
        updateUi();
        setFocus();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::stopDescriptionBarLabelEdit()
{
    if( hasDescriptionBarLabelEditionPending() )
    {
        x->setLabelForDescriptionBar(mEditingDescriptionBarLabel, mpDescriptionBarLabelEdit->text() );
        setBarAsDirty( mEditingDescriptionBarLabel, true );
        mEditingDescriptionBarLabel = -1;
        updateUi();
        setFocus();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::stopLineTextEdit()
{
    if( hasLineEditionPending() )
    {
        x->setLineText( mEditingLineIndex, mpLineTextEdit->text() );
        mEditingLineIndex = -1;
        //on met la barre dirty pour forcer le updatePageLayout
        setBarAsDirty( 0, true );
        updateUi();
        setFocus();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::stopParentheseEdit()
{
    if( hasParenthesisEditionPending() )
    {
        x->setNumberOfRepetitionForParenthesis(mEditingParentheseIndex, mpParenthesisEdit->value());
        mEditingParentheseIndex = -1;
        //on met la barre dirty pour forcer le updatePageLayout
        setBarAsDirty( 0, true );
        updateUi();
        setFocus();
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::stopTitleEdit()
{
    if( hasTitleEditionPending() )
    {
        x->setTitle( mpTitleEdit->text() );
        mEditingTitle = false;
        //on met la barre dirty pour forcer le updatePageLayout
        setBarAsDirty( 0, true );
        updateUi();
        setFocus();
    }
}

//-----------------------------------------------------------------------------
QString PartitionViewer::strokeToString( strokeType iSt ) const
{
    QString r;
    switch (iSt)
    {
        case stDa: r = "\x7c"; break;
        case stRa: r = "\xE2\x80\x95"; break;
        case stDiri: r = "\xE2\x88\xA7"; break;
        case stNone: break;
        default: break;
    }
    return r;
}
//-----------------------------------------------------------------------------
void PartitionViewer::timerEvent(QTimerEvent* ipE)
{
    if( ipE->timerId() == mCurrentBarTimerId )
    {
        updateUi();
        if( mCurrentBarTimer.elapsed() > kBarFadeDuration )
        {
            killTimer(mCurrentBarTimerId);
            mCurrentBarTimerId = 0;
        }
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::toggleDebugMode()
{
    mDebugMode = (debugMode)((mDebugMode+1) % numberOfDebugMode);
    updateUi();
}
//-----------------------------------------------------------------------------
int PartitionViewer::toPageIndex( QPoint iP ) const
{
    int r = -1;
    for( int i = 0; i < getNumberOfPages() + 1; ++i )
    {
        if( getPageRegion( prPage, i ).contains( iP ) )
        { r = i; break; }
    }
    return r;
}
//-----------------------------------------------------------------------------
vector<NoteLocator> PartitionViewer::toNoteLocator(
                                                   const vector< pair<int, int> > iV ) const
{
    vector<NoteLocator> r;
    for( int i = 0; i < iV.size(); ++i )
    { r.push_back( NoteLocator( iV[i].first, iV[i].second ) ); }
    return r;
}
//-----------------------------------------------------------------------------
/* voir la doc de Qt (http://doc.qt.io/qt-5/qrect.html#details) pour le +1 après
 chaque fonction right() et bottom()*/
void PartitionViewer::updateBar( int iBar )
{
    const int kMinBarWidth =  x->getNumberOfNotesInBar( iBar ) ? 0 : 40;
    
    Bar& b = getBar(iBar);
    
    if(iBar < x->getNumberOfDescriptionBars())
    { b.mBarType = Bar::btDescription; }
    
    //definition des rects des mots et notes.
    splitInWords(iBar);
    
    //--- Rectangle contenant la bar
    int barWidth = getBarRegion( brNoteStartX );
    foreach( QRectF r, b.mWordLayouts )
    { barWidth += r.width(); }
    
    //On ajuste la largeur pour contenir le texte.
    const int kBarPad = 2;
    barWidth = max( barWidth + kBarPad,
                   getBarRegion( brTextX ) +
                   QFontMetrics(mBarTextFont).width( x->getBarText(iBar) ) + 5 );
    QRect barRect( QPoint(0, 0), QSize(std::max( barWidth, kMinBarWidth),
                                       getBarHeight(b.mBarType) ) );
    b.mRect = barRect;
    
    //--- Rectangle contenant les beat groups.
    b.mMatraGroupsRect.clear();
    for( int i = 0; i < x->getNumberOfMatraInBar(iBar); ++i )
    {
        int left = numeric_limits<int>::max();
        int right = numeric_limits<int>::min();
        for( int j = 0; j < x->getNumberOfNotesInMatra(iBar, i); ++j )
        {
            int noteIndex = x->getNoteIndexFromMatra(iBar, i, j);
            left = std::min( left, (int)getBar(iBar).getNoteRect(noteIndex).left() );
            right = std::max( right, (int)getBar(iBar).getNoteRect(noteIndex).right() );
        }
        b.mMatraGroupsRect.push_back( QRect( QPoint( left, getBarRegion( brMatraGroupY ) ),
                                            QSize( right - left, kMatraHeight ) ) );
    }
    
    //--- le text de la barre
    QFontMetrics fm( mBarTextFont );
    QRect rect = fm.boundingRect( "+" );
    int w = max( rect.width(), rect.height() );
    rect.setTopLeft( QPoint(0, 0) );
    rect.setSize( QSize(w, w) );
    if( !x->getBarText(iBar).isEmpty() )
    { rect.setSize( fm.boundingRect( x->getBarText( iBar ) ).size() ); }
    rect.translate( getBarRegion( brTextX ), getBarRegion( brTextY ) );
    b.mTextRect = rect;
}
//-----------------------------------------------------------------------------
void PartitionViewer::updateBarLayout()
{
    setNumberOfPage( 1 );
    mBarsPerPage.clear();
    
    mLayoutCursor = getPageRegion( prBody, 0 ).topLeft();
    for( int i = x->getNumberOfDescriptionBars(); i < x->getNumberOfBars(); ++i )
    {
        Bar& b = getBar(i);
        b.mScreenLayout = QRect();
        b.mNoteScreenLayouts.clear();
        b.mWordScreenLayouts.clear();
        b.mIsWayTooLong = false;
        
        int pageIndex = toPageIndex( mLayoutCursor );
        QRect pageBody = getPageRegion(  prBody, pageIndex );
        //Si cest le debut d'une ligne, on saute 1.x ligne, sauf si
        //cest la ligne 0
        if( x->isStartOfLine( i ) && x->findLine( i ) != -1)
        {
            int lineIndex = x->findLine( i );
            int verticalShift = lineIndex != 0 ? 1.05*getBarHeight(b.mBarType) : 0;
            if( !(x->getLineText( lineIndex ).isEmpty()) ||
               mEditingLineIndex == lineIndex )
            { verticalShift += QFontMetrics(mLineFont).height(); }
            mLayoutCursor.setX( pageBody.left() );
            mLayoutCursor.setY( mLayoutCursor.y() + verticalShift );
        }
        
        //--- le layout des bars
        QRect fullLayout = b.mRect;
        while( fullLayout.width() )
        {
            pageIndex = toPageIndex( mLayoutCursor );
            pageBody = getPageRegion(  prBody, pageIndex );
            
            QRect pageLayout = fullLayout;
            pageLayout.translate( mLayoutCursor );
            
            /*Le layout depasse dans le bas de la page... on ajoute une page*/
            if( !pageBody.contains( pageLayout ) &&
               !pageBody.contains( pageLayout.bottomLeft() ) )
            {
                addPage();
                pageIndex++;
                mLayoutCursor = getPageRegion( prBody, pageIndex ).topLeft();
            }
            //ne rentre pas dans le reste de la ligne, mais peu rentrer à l'écran,
            //on saute une ligne...
            else if( !pageBody.contains( pageLayout ) &&
                    pageLayout.width() < pageBody.width() )
            {
                mLayoutCursor.setX( pageBody.left() );
                mLayoutCursor.setY( mLayoutCursor.y() + getBarHeight(b.mBarType) );
            }
            //ne rentre pas dans ce qui reste et est trop grand de toute facon...
            //En ce moment ( 14 avril 2015 ), ce n'est pas supporté, donc on
            //coupe le layout au bout de la page et on set un flag qui mettra
            //un message a l'usage lors du rendu ( voir méthode drawBar() ).
            else if( !pageBody.contains( pageLayout ) )
            {
                //on coupe le layout au bout de la page
                fullLayout.setWidth( pageBody.width() - 1 );
                b.mIsWayTooLong = true;
            }
            else //le layout de la barre entre à l'écran sans problème
            {
                //on fait le layout des notes restantes pour cette barre
                for( int j = 0; j < x->getNumberOfNotesInBar(i); ++j )
                {
                    QRectF n = b.getNoteRect(j);
                    n.translate( pageLayout.topLeft() );
                    b.mNoteScreenLayouts.push_back( n );
                }
                
                for(size_t j = 0; j < b.mWordLayouts.size(); ++j)
                {
                    QRectF n = b.mWordLayouts[j];
                    n.translate( pageLayout.topLeft() );
                    b.mWordScreenLayouts.push_back(n);
                }
                
                /*Il faut mettre le -1 sur le layout.width() parce que width()
                 retourne 1 de plus que la coordonnée bottomRight. Donc, un point
                 qui peut etre a l'extérieur de pageBody...*/
                mLayoutCursor += QPoint( fullLayout.width() - 1, 0 );
                fullLayout.setWidth(0);
                b.mScreenLayout = pageLayout;
                
                //textScreenLayout.
                b.mTextScreenLayout = b.mTextRect;
                b.mTextScreenLayout.translate( pageLayout.topLeft() );
                
                mBarsPerPage[ pageIndex ].push_back( i );
            }
        }
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::updateLayout()
{
    QElapsedTimer _timer;
    _timer.start();
    
    //layout du titre
    QRect l = QFontMetrics( mTitleFont ).boundingRect( x->getTitle() );
    QRect titleRegion = getRegion( rTitle );
    int dx = titleRegion.center().x() - l.width() / 2 ;
    int dy = titleRegion.center().y() - l.height() / 2 ;
    mTitleScreenLayout = l;
    mTitleScreenLayout.translate( QPoint( dx, dy ) - l.topLeft() );
    
    //layout des barres spéciale (gamme, accordage tarab, etc...)
    updateDescriptionBarLayout();
    
    //--- le layout des barres du sargam
    updateBarLayout();
    
    //--- le layout des ornements
    updateOrnementLayout();
    
    //--- le layout des repeition
    updateParenthesisLayout();
    
    //--- le layout des ligne, il vient apres le layout des barres parce
    //qu'il en est dependant
    updateLineLayout();
    
    if( hasLogTiming() )
    { getLog().log("PartitionViewer::updateLayout: %.3f ms", _timer.nsecsElapsed() / 1000000.0 ); }
}
//-----------------------------------------------------------------------------
void PartitionViewer::updateLineLayout()
{
    mLines.clear();
    mLines.resize( x->getNumberOfLines() );
    for( int i = 0; i < x->getNumberOfLines(); ++i )
    {
        Line& l = mLines[i];
        const Bar& b = getBar( x->getLineFirstBar(i) );
        QRect bl = b.mScreenLayout;
        QFontMetrics fm(mLineFont);
        
        //le hotSpot pour ajouter le texte
        bool hasText = !( x->getLineText(i).isEmpty() );
        if( !hasText )
        {
            QRect r( QPoint(), QSize( fm.width( "T" ), fm.height() ) );
            //le hot spot doit etre carre
            int m = std::max( r.width(), r.height() );
            r.setSize( QSize(m, m) );
            r.translate( bl.left() - r.width() - 5, bl.top() );
            l.mHotSpot = r;
        }else{ l.mHotSpot = QRect(); }
        
        //le rect pour le texte de la ligne
        if( hasText || mEditingLineIndex == i )
        {
            QRect r( QPoint(), QSize(
                                     std::max( fm.width( x->getLineText(i) ), 50 ), fm.height() ) );
            r.translate( bl.left(), bl.top() - r.height() - 2 );
            l.mTextScreenLayout = r;
        }else { l.mTextScreenLayout = QRect(); }
        
        //le rect du numero de ligne
        {
            QString s = QString::number( i + 1 ) + ")";
            QRect r( QPoint(), QSize( fm.width(s), fm.height() ) );
            // on translate le rect contenant la numero de ligne. La position
            // finale est 5 pix a gauche de la marge et centre sur la premiere
            // barre de la ligne.
            r.translate( bl.left() - r.width() - 5,
                        bl.top() + bl.height() / 2 - r.height() / 2 );
            l.mLineNumberRect = r;
        }
    }
}
//-----------------------------------------------------------------------------
/*Cette méthode est un peu tricky, parce que les ornements peuvent dépasser
 les frontières d'une barre. Puisque le systeme d'affichage est fait pour
 dessiner une barre à la fois, il faut tenir compte de quelle region de
 l'ornement sera dessiner dans la barre. 
 
 mFullOrnement: sert à définir la
 taille totale de l'ornement, c'est dans ce rectangle qu'il sera dessiner.
 mOffsets: sert a tenir le décalage à appliquer lorsqu'on dessine l'ornement
 pour une barre donnée.
 mDestination: sert à tenir la position (en x) à laquelle l'ornement doit
 être dessiner.
 */
void PartitionViewer::updateOrnementLayout()
{
    for( int iOrn = 0; iOrn < x->getNumberOfOrnements(); ++iOrn )
    {
        Ornement& o = mOrnements[iOrn];
        o.mOffsets.clear();
        o.mDestinations.clear();
        o.mFullOrnement = QRect();
        
        //update rects...
        vector<int> bi = x->getBarsInvolvedByOrnement( iOrn );
        o.mOffsets.resize( bi.size() );
        o.mDestinations.resize( bi.size() );
        int barIndex = -1;
        int widthAccum = 0;
        
        for( int i = 0; i < bi.size(); ++i )
        {
            barIndex = bi[i];
            int left = numeric_limits<int>::max();
            int right = numeric_limits<int>::min();
            
            /*Ici, on détermine la taille (left, right) de l'ornement dans la barre
             i.*/
            for( int j = 0; j < x->getNumberOfNotesInOrnement(iOrn); ++j )
            {
                NoteLocator nl = x->getNoteLocatorFromOrnement(iOrn, j);
                /*Si les notes de l'ornements s'applique à la barre barIndex, elles
                 contribueront à définir la taille de l'ornement dans la barre barIndex*/
                if( nl.getBar() == barIndex )
                {
                    int noteIndex = nl.getIndex();
                    left = std::min( left, (int)getBar(barIndex).getNoteRect(noteIndex).left() );
                    right = std::max( right, (int)getBar(barIndex).getNoteRect(noteIndex).right() );
                    
                    //quand c'est la derniere note de la barre et que le Ornement dépasse dnas la barre
                    //suivante, on prend la fin de la barre...
                    if( noteIndex == (x->getNumberOfNotesInBar(barIndex) - 1) && i < (bi.size() - 1) )
                    { right = getBar(barIndex).mRect.right(); }
                    
                    //quand c'Est la premiere note de la barre et que le Ornement vient de la barre
                    //précédente, le left est le début de la barre
                    if( noteIndex == 0 && i > 0 )
                    { left = getBar(barIndex).mRect.left(); }
                }
            }
            
            o.mDestinations[i] = make_pair( barIndex, left );
            o.mOffsets[i] = make_pair( barIndex, -widthAccum );
            widthAccum += right - left;
        }
        o.mFullOrnement = QRect( QPoint(0, getBarRegion( brOrnementY ) ),
                                QSize(widthAccum, kOrnementHeight) );
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::updateParenthesisLayout()
{
    mParenthesis.clear();
    Parenthesis repLayout;
    for( int i = 0; i < x->getNumberOfParenthesis(); ++i )
    {
        NoteLocator first = x->getNoteLocatorFromParenthesis(i, 0);
        NoteLocator last = x->getNoteLocatorFromParenthesis(i,
                                                            x->getNumberOfNotesInParenthesis(i) - 1 );
        
        //parenthese ouvrante
        const Bar& b1 = getBar( first.getBar() );
        {
            QPoint p;
            QRectF firstNoteRect = b1.getNoteRect( first.getIndex() );
            p.setX( firstNoteRect.left() + firstNoteRect.width() / 2.0 );
            p.setY( (getBarRegion( brNoteTopY ) + getBarRegion( brNoteBottomY )) / 2 );
            
            p -= QPoint( mBaseParenthesisRect.width(), 0 );
            QRectF r = mBaseParenthesisRect;
            r.translate( 0, -r.height() / 2.0 );
            r.translate( p );
            repLayout.mOpening = r;
            r.translate( b1.mScreenLayout.topLeft() ) ;
            repLayout.mOpeningScreenLayout = r;
        }
        
        //parenthese fermante
        const Bar& b2 = getBar( last.getBar() );
        {
            QPointF p;
            QRectF lastNoteRect = b2.getNoteRect( last.getIndex() );
            p.setX( lastNoteRect.right() - lastNoteRect.width() / 2.0 );
            p.setY( (getBarRegion( brNoteTopY ) + getBarRegion( brNoteBottomY )) / 2 );
            int width = mBaseParenthesisRect.width() + getParenthesisTextWidth(i);
            p += QPointF( width, 0 );
            QRectF r = mBaseParenthesisRect;
            r.translate( -width, -r.height() / 2.0 );
            r.translate( p );
            repLayout.mClosing = r;
            r.translate( b2.mScreenLayout.topLeft() ) ;
            repLayout.mClosingScreenLayout = r;
            
            //le texte
            QFontMetrics fm( mParenthesisFont );
            QRectF textR = fm.boundingRect( getParenthesisText( i ) );
            textR.translate( repLayout.mClosing.bottomRight() );
            repLayout.mText = textR;
            textR.translate( b2.mScreenLayout.topLeft() ) ;
            repLayout.mTextScreenLayout = textR;
        }
        
        mParenthesis.push_back( repLayout );
    }
}
//-----------------------------------------------------------------------------
void PartitionViewer::updateDescriptionBarLayout()
{
    for( int i = 0; i < x->getNumberOfDescriptionBars(); ++i )
    {
        Bar& b = getBar( i );
        b.mScreenLayout = QRect();
        b.mNoteScreenLayouts.clear();
        b.mWordScreenLayouts.clear();
        QRect l = b.mRect;
        const QPoint topLeft = getRegion(dbrBar, i).topLeft();
        
        l.translate( topLeft );
        b.mScreenLayout = l;
        //on fait le layout des notes pour cette barre
        for( int j = 0; j < x->getNumberOfNotesInBar(i); ++j )
        {
            QRectF n = b.getNoteRect(j);
            n.translate( topLeft );
            b.mNoteScreenLayouts.push_back(n);
        }
        
        for(size_t j = 0; j < b.mWordLayouts.size(); ++j)
        {
            QRectF n = b.mWordLayouts[j];
            n.translate( topLeft );
            b.mWordScreenLayouts.push_back(n);
        }
    }
    
    //update the position of the add button
    const QRect lastBar = getRegion( rAddRemoveDescriptionBar );
    const QPoint p( lastBar.left(), lastBar.top() );
    
    mAddDescriptionBarButtonPos = p;
}
//-----------------------------------------------------------------------------
void PartitionViewer::updateUi()
{
    if(hasModifications())
    { setWindowModified(true); }
    
    /*
     */
    bool shouldUpdateLayout = false;
    for( int i = 0; i < x->getNumberOfBars(); ++i )
    {
        Bar& b = getBar(i);
        if( b.mIsDirty )
        {
            updateBar( i );
            b.mIsDirty = false;
            shouldUpdateLayout = true;
        }
    }
    if( shouldUpdateLayout ){ updateLayout(); }
    
    //placer le ui l'écran
    //titre
    if( hasTitleEditionPending() )
    {
        QPoint p = mTitleScreenLayout.topLeft() + QPoint(-1, 5);
        p = mapToParent(p);
        mpTitleEdit->move(p);
        mpTitleEdit->show();
    }
    else{ mpTitleEdit->hide(); }
    
    //place le line edit pour les barres de description
    if( hasDescriptionBarLabelEditionPending() )
    {
        QPoint p = getRegion(dbrLabel, mEditingDescriptionBarLabel).topLeft();
        p += QPoint(-1, 5);
        //voir la creation de mpDescriptionBarLabelEdit pour explication mapping.
        p = mapToParent(p);
        mpDescriptionBarLabelEdit->move( p );
        mpDescriptionBarLabelEdit->show();
    }
    else { mpDescriptionBarLabelEdit->hide(); }
    
    //place le line edit pour les barres
    if( hasBarTextEditionPending() )
    {
        QPoint p = getBar(mEditingBarText).mTextScreenLayout.topLeft();
        p += QPoint(-1, 2);
        //voir la creation de mpLineTextEdit pour explication mapping.
        p = mapToParent(p);
        mpBarTextEdit->move( p );
        mpBarTextEdit->show();
    }else { mpBarTextEdit->hide(); }
    
    //place le bouton pour ajouter le texte aux lignes
    const int currentLine = x->findLine( getCurrentBar() );
    if( currentLine != -1 && !hasLineEditionPending() &&
       x->getLineText(currentLine).isEmpty() )
    {
        const Line& l = mLines[ currentLine ];
        const QPoint p = l.mHotSpot.topLeft();
        mpAddLineTextButton->move( p );
        mpAddLineTextButton->show();
    }
    else{ mpAddLineTextButton->hide(); }
    
    //on dessine le bar text hot spot de la barre courante si la barre est
    //courante et qu'il n'y a pas de texte
    if( !hasBarTextEditionPending() &&
       getCurrentBar() >= x->getNumberOfDescriptionBars() &&
       x->getBarText( getCurrentBar() ).isEmpty() )
    {
        const QPoint p = getBar( getCurrentBar() ).mTextScreenLayout.topLeft();
        mpAddBarTextButton->move( p );
        mpAddBarTextButton->show();
    }
    else { mpAddBarTextButton->hide(); }
    
    //place la lineEdit pour les ligne
    if( hasLineEditionPending() )
    {
        QPoint p = mLines[mEditingLineIndex].mTextScreenLayout.topLeft();
        p += QPoint(-1, 3);
        //voir la creation de mpLineTextEdit pour explication mapping.
        p = mapToParent(p);
        mpLineTextEdit->move( p );
        mpLineTextEdit->show();
    }
    else { mpLineTextEdit->hide(); }
    
    //place le spinBox pour edition des parenthese
    if(hasParenthesisEditionPending())
    {
        QPointF p = mParenthesis[mEditingParentheseIndex].mTextScreenLayout.topLeft();
        QPoint p2(p.x(), p.y());
        p2 += QPoint(-1, 2);
        p2 = mapToParent(p2);
        mpParenthesisEdit->move( p2 );
        mpParenthesisEdit->show();
        mpParenthesisEdit->selectAll();
    }
    else { mpParenthesisEdit->hide(); }
    
    //place le bouton pour ajouter les barres de descriptions
    mpAddRemoveDescriptionBar->move( mAddDescriptionBarButtonPos );
    
    /* On redimensionne le widget pour garantir que toutes les pages seront
     affichees. */
    QRect p = getRegion( rPartition );
    resize( p.width()+1, p.height()+1 );
    
    update();
}
//-----------------------------------------------------------------------------
void PartitionViewer::undoActivated()
{
    mUndoRedoStack.undo();
    setAsModified(true);
}
//-----------------------------------------------------------------------------
// --- PARTITIONVIEWER::Bar (vibhag)
//-----------------------------------------------------------------------------
QRectF PartitionViewer::Bar::getNoteRect( int iNoteIndex ) const
{
    QRectF r;
    if( iNoteIndex >= 0 && iNoteIndex < mNotesRect.size() )
    { r = mNotesRect[iNoteIndex]; }
    return r;
}
//-----------------------------------------------------------------------------
// --- PARTITIONVIEWER::Ornement
//-----------------------------------------------------------------------------
int PartitionViewer::Ornement::getDestination( int iBar ) const
{
    int r = 0;
    for( int i = 0; i < mDestinations.size(); ++i )
    { if( mDestinations[i].first == iBar ) { r = mDestinations[i].second; break; } }
    return r;
}
//-----------------------------------------------------------------------------
int PartitionViewer::Ornement::getOffset( int iBar ) const
{
    int r = 0;
    for( int i = 0; i < mOffsets.size(); ++i )
    { if( mOffsets[i].first == iBar ) { r = mOffsets[i].second; break; } }
    return r;
}
