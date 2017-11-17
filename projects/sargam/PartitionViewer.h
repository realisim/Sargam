

#ifndef PartitionViewer_hh
#define PartitionViewer_hh

#include "data.h"
#include <QLineEdit>
#include <QPrinter>
#include <QtWidgets>
#include <QSettings>
#include "utils/Command.h"
#include "utils/CommandStack.h"
#include "utils/Log.h"
#include <vector>

namespace realisim
{
namespace sargam
{
  
class ThinLineEdit : public QLineEdit
{ public: ThinLineEdit( QWidget* = 0 ); };
  
//------------------------------------------------------------------------------
/*
 
 Expliquer le system de undo/redo. commandXxx qui est publique et
 l'implémentation privé doCommandXxx
 */
class PartitionViewer : public QWidget
{
  Q_OBJECT
public:
  PartitionViewer( QWidget* );
  ~PartitionViewer();
  
  void commandAddBar();
  void commandAddGraceNotes();
  void commandAddLine();
  void commandAddMatra();
  void commandAddNote( Note );
  void commandAddOrnement( ornementType );
  void commandAddParenthesis( int );
  void commandAddStroke( strokeType );
  void commandBreakMatrasFromSelection();
  void commandBreakOrnementsFromSelection();
  void commandDecreaseOctave();
  void commandErase();
  void commandIncreaseOctave();
  void commandRemoveParenthesis();
  void commandRemoveSelectionFromGraceNotes();
  void commandRemoveStroke();
  void commandShiftNote();
QImage getBarAsImage(int) const;
  Composition getComposition() const;
  int getCurrentBar() const;
  int getCurrentNote() const;
  NoteLocator getCursorPosition() const;
  int getFontSize() const;
  int getOctave() const;
  QPageLayout::Orientation getLayoutOrientation() const;
  const utils::Log& getLog() const;
  QPageSize::PageSizeId getPageSizeId() const;
  int getNumberOfSelectedNote() const;
  NoteLocator getSelectedNote( int ) const;
  script getScript() const;
  bool hasLogTiming() const { return mHasLogTiming; }
  bool hasModifications() const;
  bool hasSelection() const;
  bool isDebugging() const;
  bool isVerbose() const {return mIsVerbose;}
  void print( QPrinter* );
  void setAsModified(bool m) {mHasModifications = m;}
  void setAsVerbose( bool );
  void setComposition( Composition* );
  void setFontSize(int);
  void setLayoutOrientation( QPageLayout::Orientation );
  void setLog( utils::Log* );
  void setLogTiming( bool iL ) {mHasLogTiming = iL;}
  void setPageSize( QPageSize::PageSizeId );
  void setScript( script );
  void toggleDebugMode();

public slots:
  void redoActivated();
  void undoActivated();
  
signals:
  void ensureVisible( QPoint );
  void interactionOccured(); //documenter...
  
protected slots:
  void addBarTextClicked();
  void addDescriptionBarClicked();
  void addLineTextClicked();
  void removeDescriptionBarClicked();
  void resizeEditToContent();
  void stopBarTextEdit();
  void stopDescriptionBarLabelEdit();
  void stopLineTextEdit();
  void stopParentheseEdit();
  void stopTitleEdit();
  
protected:
  friend class PartitionViewerCommand;
  friend class CommandAddBar;
  friend class CommandAddDescriptionBar;
  friend class CommandAddGraceNotes;
  friend class CommandAddLine;
  friend class CommandAddMatra;
  friend class CommandAddNote;
  friend class CommandAddOrnement;
  friend class CommandAddParenthesis;
  friend class CommandAddStroke;
  friend class CommandBreakMatrasFromSelection;
  friend class CommandBreakOrnementsFromSelection;
  friend class CommandDecreaseOctave;
  friend class CommandErase;
  friend class CommandIncreaseOctave;
  friend class CommandRemoveDescriptionBar;
  friend class CommandRemoveParenthesis;
  friend class CommandRemoveSelectionFromGraceNotes;
  friend class CommandRemoveStroke;
  friend class CommandShiftNote;
  
  enum region { rPartition, rTitle, rDescriptionBars,
    rAddRemoveDescriptionBar};
  enum pageRegion { prPage, prBody, prPageFooter };
  enum barRegion { brNoteStartX, brNoteTopY, brNoteBottomY, brStrokeY,
    brOrnementY, brMatraGroupY, brGraceNoteTopY, brGraceNoteBottomY,
    brTextX, brTextY, brLowerOctaveY, brUpperOctaveY, brGraceNoteLowerOctaveY,
    brGraceNoteUpperOctaveY, brKomalLineY, brGraceNoteKomalLineY };
  enum descriptionBarRegion { dbrLabel, dbrBar };
  enum colors{ cHover, cSelection };
  enum debugMode{ dmNone = 0, dmNoteLayout, dmWordLayout, dmBarInfo,
    numberOfDebugMode };
  
  /*Le type de barre indique s'il s'agit d'une barre de dexcription, comme les
    barres dans le haut de la page pour indiquer la gamme et l'accodage, ou s'il
    s'agit d'une barre normal de partition. */
  struct Bar
  {
    enum barType {btNormal = 0, btDescription};
    Bar() : mBarType(btNormal), mIsDirty(true), mIsWayTooLong(false){;}
    QRectF getNoteRect( int ) const;
    
    /*--- cache d'Affichage 
     mRect: coin à (0, 0). C'est les rect qui contient toute la barre
     mNotesRect: les rect qui contiennent chaque note en coordonnées barre. Donc,
       par rapport à (0, 0).
     mMatraGroupRect: les rects qui contiennent les matras. En coordonnées barre.
     mScreenLayout: mRect, mais en coordonnées écran.
     mNoteScreenLayouts: comme mScreenLayout mais pour les notes.
     mTextScreenLayout: le rect qui contient le texte de la barre. En 
        coordonnées écran.
     mBartype: type de barre.
     mIsDirty:
     mIsWayTooLong:
     */
    QRect mRect;
    std::vector< QRectF > mNotesRect;
    std::vector< QRect > mMatraGroupsRect;
    QRect mScreenLayout;
    std::vector< QRectF > mNoteScreenLayouts;
    QRect mTextRect; //bar coord.
    QRect mTextScreenLayout;
    std::vector< std::pair<QString, bool> > mWords;
    std::vector<QRectF> mWordLayouts;
std::vector<QRectF> mWordScreenLayouts; //pas vraiment besoin autre que pour le debogage...
    barType mBarType;
    bool mIsDirty;
    bool mIsWayTooLong;
  };
  
  struct Ornement
  {
    Ornement() {;}

    int getDestination( int ) const;
    int getOffset( int ) const;
    
    /*--- cache d'Affichage 
     Coordonées barre:
     mFullOrnement est le rectangle avec coin à (0, 0) qui contient
       l'ornement complet, même s'il est sur plusieurs barres.
     mOffsets: l'offset X à appliquer à l'ornement
     mDestination: la coordonnée X de destination de l'ornement dans la barre
     */
    QRect mFullOrnement;
    std::vector< std::pair< int, int > > mOffsets;
    std::vector< std::pair< int, int > > mDestinations;
  };
  
  struct Line
  {
    Line() {;}
    
    /*--- cache d'affichage 
     Coordonnées écran. */
    QRect mLineNumberRect;
    QRect mTextScreenLayout;
    QRect mHotSpot;
  };
  
  struct Parenthesis
  {
    Parenthesis() {;}
    
    QRectF mOpening; //coordonnee bar
    QRectF mClosing; //coordonnee bar
    QRectF mText; //coordonnee bar
    QRectF mOpeningScreenLayout; //coordonnee ecran
    QRectF mClosingScreenLayout; //coordonnee ecran
    QRectF mTextScreenLayout; //coordonnee ecran
  };
  
  //information relatives aux selections par la souris.
  //On decris aussi l'État de la souris.
  enum MouseState {msIdle, msPressed, msDraging};
  struct MouseSelection
  {
    MouseSelection() : mStart(), mEnd(), mPixelWhenPressed() {}
    
    NoteLocator mStart;
    NoteLocator mEnd;
    
    QPoint mPixelWhenPressed;
  };

  void addBar( int );
  void addNoteToSelection( int, int );
  void addPage();
  Note alterNoteFromScale( Note ) const;
  void clear();
  void clearSelection();
  void createUi();
  int cmToPixel( double ) const;
  void doCommandAddBar();
  void doCommandAddDescriptionBar();
  void doCommandAddGraceNotes();
  void doCommandAddLine();
  void doCommandAddMatra();
  void doCommandAddNote(Note);
  void doCommandAddOrnement( ornementType );
  void doCommandAddParenthesis( int );
  void doCommandAddStroke( strokeType );
  void doCommandBreakMatrasFromSelection();
  void doCommandBreakOrnementsFromSelection();
  void doCommandDecreaseOctave();
  void doCommandErase();
  void doCommandIncreaseOctave();
  void doCommandRemoveDescriptionBar();
  void doCommandRemoveParenthesis();
  void doCommandRemoveSelectionFromGraceNotes();
  void doCommandRemoveStroke();
  void doCommandShiftNote();
  void draw( QPaintDevice* iPaintDevice, QRect ) const;
  void drawBar( QPainter*, int ) const;
  void drawBarContour( QPainter*, int, QColor ) const;
  void drawCursor( QPainter* ) const;
  void drawGamak( QPainter*, QRect, double ) const;
  void drawLine( QPainter*, int ) const;
  void drawPageFooter( QPainter*, int ) const;
  void drawPageFooters( QPainter* iP ) const;
  void drawSelectedNotes( QPainter* ) const;
  void drawDescriptionBars( QPainter* ) const;
  void drawTitle( QPainter* ) const;
  void eraseBar(int);
  void eraseOrnement( int );
  Bar& getBar(int);
  const Bar& getBar(int) const;
  int getBarHeight(Bar::barType) const;
  int getBarRegion( barRegion, Bar::barType = Bar::btNormal ) const;
  std::vector<int> getBarsFromPage( int ) const;
  QColor getColor( colors ) const;
  QLineF getCursorLine() const;
  debugMode getDebugMode() const;
  QString getInterNoteSpacingAsQString(NoteLocator, NoteLocator) const;
  utils::Log& getLog();
  MouseState getMouseState() const {return mMouseState;}
  NoteLocator getNext( const NoteLocator& ) const;
  NoteLocator getNoteLocatorAtPosition(QPoint) const;
  int getNumberOfPages() const;
QRect getPageRegion( pageRegion ) const; //getRegion
QRect getPageRegion( pageRegion, int ) const; //getRegion
  QSizeF getPageSizeInInch() const;
  NoteLocator getPrevious(const NoteLocator&) const;
  QRect getRegion( region ) const;
  QRect getRegion( descriptionBarRegion, int ) const;
  QString getParenthesisText( int ) const;
  int getParenthesisTextWidth( int ) const;
  bool hasBarTextEditionPending() const;
  bool hasDescriptionBarLabelEditionPending() const;
  bool hasLineEditionPending() const;
  bool hasParenthesisEditionPending() const;
  bool hasTitleEditionPending() const {return mEditingTitle;}
  bool isNoteSelected( int, int ) const;
  virtual void keyPressEvent( QKeyEvent* ) override;
  virtual void keyReleaseEvent( QKeyEvent* ) override;
  virtual void mouseMoveEvent( QMouseEvent* ) override;
  virtual void mousePressEvent(QMouseEvent*) override;
  virtual void mouseReleaseEvent( QMouseEvent* ) override;
  void moveGraceNoteBackward( int, int );
  void moveGraceNoteForward( int, int );
  void moveMatraBackward( int, int );
  void moveMatraForward( int, int );
  void moveOrnementBackward( int, int );
  void moveOrnementForward( int, int );
  void moveParenthesisBackward( int, int );
  void moveParenthesisForward( int, int );
  void moveStrokeBackward( int, int );
  void moveStrokeForward( int, int );
  QString noteToString( Note ) const;
  virtual void paintEvent(QPaintEvent*);
  void resetComposition(Composition*);
  void resizeLineEditToContent(QLineEdit*);
  void resizeSpinBoxToContent(QSpinBox*);
  void setAllBarsAsDirty(bool);
  void setBarAsDirty( int, bool );
  void setBarsAsDirty( std::vector<int>, bool );
  void setCurrentBar(int);
  void setCurrentNote(int);
  void setCursorPosition( NoteLocator );
  void setMouseState(MouseState ms) { mMouseState = ms; }
  void setNumberOfPage(int);
  virtual void showEvent(QShowEvent*) override;
  std::map< int, std::vector< int > > splitPerBar( std::vector< std::pair<int, int> > ) const;
  void splitInWords(int); //makeNoteRect
  void startBarTextEdit( int );
  void startDescriptionBarLabelEdit( int );
  void startParentheseEdit( int );
  void startLineTextEdit( int );
  void startTitleEdit();
  QString strokeToString( strokeType ) const;
  virtual void timerEvent(QTimerEvent*);
  int toPageIndex( QPoint ) const;
  std::vector<NoteLocator> toNoteLocator( const std::vector< std::pair<int, int> > ) const;
  void updateBar( int );
  void updateBarLayout();
  void updateOrnementLayout();
  void updateParenthesisLayout();
  void updateLayout();
  void updateLineLayout();
  void updateDescriptionBarLayout( );
  void updateUi();
  
  //--- ui
  ThinLineEdit* mpTitleEdit;
  ThinLineEdit* mpLineTextEdit;
  ThinLineEdit* mpBarTextEdit;
  ThinLineEdit* mpDescriptionBarLabelEdit;
  QSpinBox* mpParenthesisEdit;
  QWidget* mpAddRemoveDescriptionBar;
  QPushButton* mpAddLineTextButton;
  QPushButton* mpAddBarTextButton;
  
  //--- data
  debugMode mDebugMode;
  QRect mTitleScreenLayout;
  QPageSize::PageSizeId mPageSizeId;
  QPageLayout::Orientation mLayoutOrientation;
  int mNumberOfPages;
  QFont mTitleFont;
  QFont mBarFont;
  QFont mBarTextFont;
  QFont mGraceNotesFont;
  QFont mLineFont;
  QFont mStrokeFont;
  QFont mParenthesisFont;
  QRectF mBaseParenthesisRect;
  std::vector< Bar > mBars;
  std::vector< Ornement > mOrnements;
  std::vector< Line > mLines;
  std::map< int, std::vector<int> > mBarsPerPage;
  std::vector< Parenthesis > mParenthesis;
  QPoint mAddDescriptionBarButtonPos;
  int mCurrentBar;
  int mCurrentBarTimerId;
  QTime mCurrentBarTimer;
  int mCurrentNote;
  QPoint mLayoutCursor;
  std::vector< std::pair<int, int> > mSelectedNotes; //bar, index
  int mEditingBarText;
  int mEditingDescriptionBarLabel;
  int mEditingLineIndex;
  int mEditingParentheseIndex;
  bool mEditingTitle;
  int mBarHoverIndex;
  int mDescriptionBarLabelHoverIndex;
  static Composition mDummyComposition;
  Composition* x; //jamais null...
  utils::Log mDefaultLog;
  utils::Log *mpLog; //jamais null
  bool mIsVerbose;
  bool mHasLogTiming;
  static Bar mDummyBar;
  script mScript;
  utils::CommandStack mUndoRedoStack;
  bool mHasModifications;
  
  MouseState mMouseState;
  MouseSelection mMouseSelection;
};

  
}
}

#endif
