//
//  data.cpp

#include "data.h"
#include <QDataStream>

using namespace std;
using namespace realisim;
using namespace sargam;

namespace
{
    const int kMagicHeader = 0x00ABEF54;
    
    const int kCurrentVersion = 6;
    /*version 3: ajout du texte sur les barres.
     version 4: ajout des parentheses de repetition
     version 5: ajout des barres de description arbitraires (en nombre)
     version 6: ajouts des Options de composition. 
        mShowLineNumber.
     */
    
    /*kLowestSupportedVersion indique la plus basse version de logiciel capable
     de lire ce format de fichier. C'est pour les vielles versions, il est
     possible, qu'une vielle version puisse lire le nouveau format. */
    const int kLowestSupportedVersion = 3;
}

//------------------------------------------------------------------------------
//--- Note
//------------------------------------------------------------------------------
Note::Note( noteValue iV) :
mValue( iV ),
mOctave( 0 ),
mModification( nmShuddh )
{}
//------------------------------------------------------------------------------
Note::Note( noteValue iV, int iO, noteModification iM ) :
mValue( iV ),
mOctave( iO ),
mModification( iM )
{}
//------------------------------------------------------------------------------
bool Note::canBeKomal() const
{
    bool r = false;
    if( mModification != nmKomal &&
       (mValue == nvRe || mValue == nvGa || mValue == nvDha || mValue == nvNi ) )
    { r = true; }
    return r;
}
//------------------------------------------------------------------------------
bool Note::canBeTivra() const
{
    bool r = false;
    if( mModification != nmTivra && mValue == nvMa ){ r = true; }
    return  r;
}
//------------------------------------------------------------------------------
noteModification Note::getModification() const
{ return mModification; }
//------------------------------------------------------------------------------
int Note::getOctave() const
{ return mOctave; }
//------------------------------------------------------------------------------
noteValue Note::getValue() const
{ return mValue; }
//------------------------------------------------------------------------------
void Note::setModification( noteModification iM )
{ mModification = iM; }
//------------------------------------------------------------------------------
void Note::setOctave( int iO )
{ mOctave = std::min( std::max(iO, -1), 1 ); }
//------------------------------------------------------------------------------
void Note::setValue( noteValue iV )
{ mValue = iV; }

//-----------------------------------------------------------------------------
// --- Stroke
//-----------------------------------------------------------------------------
Stroke::Stroke( strokeType iSt, int iNoteIndex ) :
mStrokeType( iSt ),
mSpan( 1, iNoteIndex )
{}
//-----------------------------------------------------------------------------
Stroke::Stroke( strokeType iSt, vector<int> iSpan ) :
mStrokeType( iSt ),
mSpan( iSpan )
{}

//------------------------------------------------------------------------------
//--- NoteLocator
//------------------------------------------------------------------------------
NoteLocator::NoteLocator() :
mBar( -1 ),
mIndex( -1 )
{}
//------------------------------------------------------------------------------
NoteLocator::NoteLocator( int iBar, int iIndex ) :
mBar( iBar ),
mIndex( iIndex )
{}
//------------------------------------------------------------------------------
int NoteLocator::getBar() const
{ return mBar; }
//------------------------------------------------------------------------------
int NoteLocator::getIndex() const
{ return mIndex; }
//------------------------------------------------------------------------------
bool NoteLocator::isValid() const
{ return mBar != -1; }
//------------------------------------------------------------------------------
NoteLocator NoteLocator::invalid()
{ return NoteLocator(-1, -1); }
//------------------------------------------------------------------------------
bool NoteLocator::operator<( const NoteLocator& iRhs ) const
{
    bool r = this->getBar() < iRhs.getBar();
    if( this->getBar() == iRhs.getBar() )
    { r = this->getIndex() < iRhs.getIndex(); }
    return r;
}
//------------------------------------------------------------------------------
bool NoteLocator::operator<=( const NoteLocator& iRhs ) const
{
    bool r = this->getBar() < iRhs.getBar();
    if( this->getBar() == iRhs.getBar() )
    { r = this->getIndex() <= iRhs.getIndex(); }
    return r;
}
//------------------------------------------------------------------------------
bool NoteLocator::operator>( const NoteLocator& iRhs ) const
{
    bool r = this->getBar() > iRhs.getBar();
    if( this->getBar() == iRhs.getBar() )
    { r = this->getIndex() > iRhs.getIndex(); }
    return r;
}
//------------------------------------------------------------------------------
bool NoteLocator::operator>=( const NoteLocator& iRhs ) const
{
    bool r = this->getBar() > iRhs.getBar();
    if( this->getBar() == iRhs.getBar() )
    { r = this->getIndex() >= iRhs.getIndex(); }
    return r;
}
//------------------------------------------------------------------------------
bool NoteLocator::operator==( const NoteLocator& iRhs ) const
{ return getBar() == iRhs.getBar() && getIndex() == iRhs.getIndex(); }
//------------------------------------------------------------------------------
bool NoteLocator::operator!=( const NoteLocator& iRhs ) const
{ return !operator==(iRhs); }
//------------------------------------------------------------------------------
//--- Composition :: Ornement
//------------------------------------------------------------------------------
Composition::Ornement::Ornement()
{}
Composition::Ornement::Ornement( ornementType iOt, vector<NoteLocator> iNotes ):
mOrnementType( iOt ),
mNotes( iNotes )
{}

//------------------------------------------------------------------------------
//--- Composition :: Line
//------------------------------------------------------------------------------
Composition::Line::Line()
{}
Composition::Line::Line( int iFirstBar, QString iText ):
mFirstBar( iFirstBar ),
mText( iText )
{}

//------------------------------------------------------------------------------
bool Composition::Line::operator<( const Composition::Line& iRhs ) const
{ return this->mFirstBar < iRhs.mFirstBar; }

//------------------------------------------------------------------------------
//--- Composition :: Parenthesis
//------------------------------------------------------------------------------
Composition::Parenthesis::Parenthesis() : mNumber( 1 )
{}
Composition::Parenthesis::Parenthesis( vector<NoteLocator> iNotes, int iN ) :
mNotes( iNotes ),
mNumber( iN )
{}

//------------------------------------------------------------------------------
//--- Composition :: Options
//------------------------------------------------------------------------------
Composition::Options::Options() : mShowLineNumber(true)
{}

//------------------------------------------------------------------------------
//--- Composition
//------------------------------------------------------------------------------
Composition::Bar Composition::mDummyBar;
realisim::sargam::Note Composition::mDummyNote(nvSa);

Composition::Composition()
{
    setTitle( "Untitled" );
    
    //Par defaut, on ajoute 2 barres de description.
    addDescriptionBar("Scale: ");
    mBars[0].mNotes = defaultScale();
    
    addDescriptionBar("Tarab tuning: ");
    mBars[1].mNotes = defaultTarabTuning();
    
    //on ajoute une barre vide pour bien commencer et la première
    //ligne commence avec la barre 2.
    addBar();
    addLine(2);
}
//------------------------------------------------------------------------------
void Composition::addError( QString iE ) const
{ mErrors += mErrors.isEmpty() ? iE : "\n" + iE; }
//------------------------------------------------------------------------------
/*Ajoute une barre a la fin*/
void Composition::addBar()
{ addBar( getNumberOfBars() ); }
//------------------------------------------------------------------------------
/*Ajoute une barre immédiatement après iBarIndex*/
void Composition::addBar( int iBarIndex )
{
    vector<Bar>::iterator it = iBarIndex == mBars.size() ? mBars.end() : mBars.begin() + (iBarIndex + 1);
    if( it < mBars.end() )
    {
        mBars.insert(it, Bar());
        shiftLines( 1, iBarIndex );
        shiftOrnements( 1, iBarIndex );
        shiftParenthesis(1, iBarIndex );
    }
    else{ mBars.push_back( Bar() ); }
}
//------------------------------------------------------------------------------
void Composition::addDescriptionBar(QString iLabel)
{
    addBar( getNumberOfDescriptionBars() - 1);
    mDescriptionBarLabels.push_back( iLabel );
}
//------------------------------------------------------------------------------
/*ajoute un saut de ligne ' la barre iIbarIndex avec le texte iText*/
void Composition::addLine( int iBarIndex, QString iText /*= QString*/ )
{
    /*Si la barre n'est pas deja un debut de ligne, on l'ajoute.*/
    if( !isStartOfLine( iBarIndex ) )
    {
        mLines.push_back( Line( iBarIndex, iText ) );
        /*les lignes sont trie en ordre croissant de iBarIndex*/
        sort( mLines.begin(), mLines.end() );
    }
}
//------------------------------------------------------------------------------
void Composition::addMatra( int iBar, std::vector<int> iNoteIndices )
{
    if( !iNoteIndices.empty() )
    {
        Bar& b = getBar( iBar );
        b.mMatras.push_back( iNoteIndices );
    }
}
//------------------------------------------------------------------------------
/*Ajoute une note à la fin de la bar iBarIndex*/
void Composition::addNote( int iBarIndex, Note iN  )
{ addNote( iBarIndex, getNumberOfNotesInBar(iBarIndex)-1, iN ); }
//------------------------------------------------------------------------------
/*Ajoute la note iNote dans la barre iBar immédiatement après la note en
 position iNoteIndex.
 L'intégrité des données (matras, notes de graces, strokes, ornements et
 lignes) sera conservée. De plus, si on ajoute une note au milieu d'un matra,
 la note sera ajouté au matra. Il en va de même pour note de grace, stroke et
 ornements. */
void Composition::addNote( int iBar, int iNoteIndex, Note iNote )
{
    int insertIndex = iNoteIndex + 1;
    vector< Note >::iterator pos = getBar(iBar).mNotes.begin() +
    insertIndex;
    
    getBar(iBar).mNotes.insert( pos, iNote );
    
    //on shift le matra de 1
    shiftMatras( 1, iBar, iNoteIndex );
    //Si la note au curseur, ainsi que la suivante, sont dans le matra, la
    //nouvelle note devra aussi etre dans le matra
    int matra1 = findMatra( iBar, iNoteIndex );
    int matra2 = findMatra( iBar, iNoteIndex + 2 );
    if( matra1 != -1 && matra1 == matra2 )
    { addNoteToMatra( matra1, iBar, insertIndex ); }
    
    //on shift les ornements de 1
    shiftOrnements( 1, iBar, iNoteIndex );
    //Si la note au curseur, ainsi que la suivante, sont dans lornement, la
    //nouvelle note devra aussi etre dans l'ornement
    int ornm1 = findOrnement( iBar, iNoteIndex );
    int ornm2 = findOrnement( iBar, iNoteIndex + 2 );
    if( ornm1 != -1 && ornm1 == ornm2 )
    { addNoteToOrnement( ornm1, iBar, insertIndex ); }
    
    
    //shift les Parenthesis de 1
    shiftParenthesis( 1, iBar, iNoteIndex );
    //Si la note au curseur, ainsi que la suivante, sont dans larep, la
    //nouvelle note devra aussi etre dans la rep.
    int rep1 = findParenthesis( iBar, iNoteIndex );
    int rep2 = findParenthesis( iBar, iNoteIndex + 2 );
    if( rep1 != -1 && rep1 == rep2  )
    { addNoteToParenthesis( rep1, iBar, insertIndex ); }
    
    
    //on shift les graces notes
    shiftGraceNotes( 1, iBar, iNoteIndex );
    //Si la note au curseur, ainsi que la suivante, sont des notes de grace, la
    //nouvelle note devra aussi etre une note de grace
    if( isGraceNote( iBar, iNoteIndex ) && isGraceNote( iBar, iNoteIndex + 2 ) )
    { addGraceNote( iBar, insertIndex ); }
    
    //on shit les strokes
    shiftStrokes( 1, iBar, iNoteIndex );
}
//------------------------------------------------------------------------------
/*Ajoute l'index iIndex dans le matra iMatra de la barre iBar.*/
void Composition::addNoteToMatra( int iMatra, int iBar, int iIndex )
{
    vector<int>& m = getBar(iBar).mMatras[iMatra];
    m.push_back( iIndex );
    sort( m.begin(), m.end() );
}
//------------------------------------------------------------------------------
/*Ajoute l'index (iBar,iIndex) dans lornement iOrn.*/
void Composition::addNoteToOrnement( int iOrn, int iBar, int iIndex )
{
    Ornement& o = mOrnements[iOrn];
    o.mNotes.push_back( NoteLocator( iBar, iIndex ) );
    sort( o.mNotes.begin(), o.mNotes.end() );
}
//------------------------------------------------------------------------------
/*Ajoute l'index (iBar,iIndex) dans lornement iOrn.*/
void Composition::addNoteToParenthesis( int iRep, int iBar, int iIndex )
{
    vector<NoteLocator>& vnl = mParenthesis[iRep].mNotes;
    vnl.push_back( NoteLocator( iBar, iIndex ) );
    sort( vnl.begin(),vnl.end() );
}
//------------------------------------------------------------------------------
void Composition::addOrnement( ornementType iOt, std::vector<NoteLocator> iNotes )
{ mOrnements.push_back( Ornement( iOt, iNotes ) ); }
//------------------------------------------------------------------------------
void Composition::addParenthesis( vector<NoteLocator> iNotes, int iNumRep )
{ mParenthesis.push_back( Parenthesis( iNotes, iNumRep ) ); }
//------------------------------------------------------------------------------
void Composition::addStroke( int iBar, strokeType iSt, int iNoteIndice )
{
    Bar& b = getBar( iBar );
    b.mStrokes.push_back( Stroke( iSt, iNoteIndice ) );
}
//------------------------------------------------------------------------------
void Composition::addStroke( int iBar, strokeType iSt, vector<int> iNoteIndices )
{
    Bar& b = getBar( iBar );
    b.mStrokes.push_back( Stroke( iSt, iNoteIndices ) );
}
//------------------------------------------------------------------------------
void Composition::addGraceNote( int iBar, int iNoteIndex )
{ getBar(iBar).mGraceNotes.push_back( iNoteIndex ); }
//------------------------------------------------------------------------------
void Composition::clear()
{
    mBars.clear();
    mDescriptionBarLabels.clear();
    mLines.clear();
    mOrnements.clear();
    mErrors.clear();
    mParenthesis.clear();
}
//------------------------------------------------------------------------------
vector<Note> Composition::defaultScale() const
{
    vector<Note> r;
    for( int i = nvSa; i <= nvNi; ++i )
    { r.push_back( Note( (noteValue)i ) ); }
    return r;
}
//------------------------------------------------------------------------------
vector<Note> Composition::defaultTarabTuning() const
{
    vector<Note> r;
    r.push_back( Note( nvSa ) );
    r.push_back( Note( nvNi, -1 ) );
    r.push_back( Note( nvSa ) );
    r.push_back( Note( nvRe ) );
    r.push_back( Note( nvGa ) );
    r.push_back( Note( nvMa ) );
    r.push_back( Note( nvPa ) );
    r.push_back( Note( nvPa ) );
    r.push_back( Note( nvDha ) );
    r.push_back( Note( nvNi ) );
    r.push_back( Note( nvSa, 1 ) );
    r.push_back( Note( nvRe, 1 ) );
    r.push_back( Note( nvGa, 1 ) );
    return r;
}
//------------------------------------------------------------------------------
/*Efface la barre iBarIndex. Si la barre est un debut de ligne, la
 ligne sera aussi effacée.*/
void Composition::eraseBar( int iBarIndex )
{
    if( iBarIndex >= 0 && iBarIndex < getNumberOfBars() )
    {
        if( isStartOfLine( iBarIndex ) )
        {
            int cl = findLine(iBarIndex);
            /*Si la barre qui suit est sur la meme ligne, cette barre devient
             le debut de la ligne.*/
            if( findLine( iBarIndex + 1 ) == cl )
            {
                eraseLine( cl );
                addLine( iBarIndex + 1 );
            }
            else{ eraseLine( cl ); }
        }
        /*on efface toutes les notes de la barres afin de bien nettoyer
         les ornements...*/
        for( int i = getNumberOfNotesInBar(iBarIndex) - 1; i >=0 ; --i )
        { eraseNote(iBarIndex, i); }
        
        mBars.erase( mBars.begin() + iBarIndex );
        shiftLines(-1, iBarIndex);
        shiftOrnements(-1, iBarIndex);
        shiftParenthesis(-1, iBarIndex );
    }
}
//------------------------------------------------------------------------------
//enleve la barre iIndex, qui doit etre une barre de description, de plus, le
//label pour la barre de description est enlevé
bool Composition::eraseDescriptionBar(int iIndex)
{
    bool r = false;
    //il est impossible d'enlever la barre 0 qui contient la gamme
    //par défaut
    if(iIndex >= 1 && iIndex < getNumberOfDescriptionBars())
    {
        eraseBar(iIndex);
        mDescriptionBarLabels.erase( mDescriptionBarLabels.begin() + iIndex );
        r = true;
    }
    return r;
}
//------------------------------------------------------------------------------
/*Efface la note de grace iNoteIndex de la narre iBarIndex.*/
void Composition::eraseGraceNote( int iBarIndex, int iNoteIndex )
{
    Bar& b = getBar( iBarIndex );
    for( int i = 0; i < b.mGraceNotes.size(); ++i )
    {
        if( b.mGraceNotes[i] == iNoteIndex )
        {
            b.mGraceNotes.erase( b.mGraceNotes.begin() + i );
            break;
        }
    }
}
//------------------------------------------------------------------------------
/*Efface la ligne iLineIndex*/
void Composition::eraseLine( int iLineIndex )
{
    if( iLineIndex >= 0 && iLineIndex < mLines.size() )
    { mLines.erase( mLines.begin() + iLineIndex ); }
}
//------------------------------------------------------------------------------
/*Efface le matra iMatra de la barre iBar.*/
void Composition::eraseMatra( int iBar, int iMatra )
{
    Bar& b = getBar(iBar);
    if( iMatra >= 0 && iMatra < getNumberOfMatraInBar(iBar) )
    { b.mMatras.erase( b.mMatras.begin() + iMatra ); }
}
//------------------------------------------------------------------------------
/*Efface la note iIndex de la barre iBar. Cette operation conserve
 l'integrite des donnees (voir methode addNote).*/
void Composition::eraseNote( int iBar, int iIndex )
{
    Bar& b = getBar(iBar);
    if( iIndex >= 0 && iIndex < getNumberOfNotesInBar(iBar) )
    {
        b.mNotes.erase( b.mNotes.begin() + iIndex );
        
        //si la note est dans le matra, on l'enleve, sinon on shift le groupe
        if( isNoteInMatra( iBar, iIndex ) )
        { eraseNoteFromMatra( iBar, iIndex ); }
        shiftMatras( -1, iBar, iIndex );
        
        //on enleve des ornements
        if( isNoteInOrnement( iBar, iIndex ) )
        { eraseNoteFromOrnement( iBar, iIndex ); }
        shiftOrnements( -1, iBar, iIndex );
        
        if( isNoteInParenthesis( iBar, iIndex ) )
        { eraseNoteFromParenthesis( iBar, iIndex ); }
        shiftParenthesis( -1, iBar, iIndex );
        
        //on eneleve les graceNotes
        if( isGraceNote( iBar, iIndex ) )
        { eraseGraceNote( iBar, iIndex ); }
        shiftGraceNotes( -1, iBar, iIndex );
        
        //on enleve le stoke
        eraseStroke( iBar, findStroke( iBar, iIndex ) );
        shiftStrokes( -1, iBar, iIndex );
    }
}
//------------------------------------------------------------------------------
/*Efface la note iIndex de la barre iBar du matra qui la contient. Si aucun
 matra ne la contient, il ne se passe rien.*/
void Composition::eraseNoteFromMatra( int iBar, int iIndex )
{
    Bar& b = getBar( iBar );
    int i = findMatra(iBar, iIndex);
    if( i >= 0 )
    {
        vector<int>& m = b.mMatras[i];
        m.erase( find( m.begin(), m.end(), iIndex ) );
        if( m.empty() ){ eraseMatra(iBar, i); }
    }
}
//------------------------------------------------------------------------------
/*voir eraseNoteFromMatra.*/
void Composition::eraseNoteFromOrnement( int iBar, int iIndex )
{
    int i = findOrnement(iBar, iIndex);
    if( i >= 0 )
    {
        Ornement& o = mOrnements[i];
        vector< NoteLocator >::iterator it = o.mNotes.begin();
        for( ; it != o.mNotes.end(); ++it )
        {
            if( (*it).getBar() == iBar && (*it).getIndex() == iIndex )
            {
                o.mNotes.erase( it );
                break;
            }
        }
        if( o.mNotes.empty() ) { eraseOrnement( i ); }
    }
}
//------------------------------------------------------------------------------
/*Efface la note iIndex de la barre iBar de la répétition qui la contient. Si
 aucune répétition ne la contient, il ne se passe rien.*/
void Composition::eraseNoteFromParenthesis( int iBar, int iIndex )
{
    int rep = findParenthesis(iBar, iIndex);
    
    vector<NoteLocator>& vnl = mParenthesis[ rep ].mNotes;
    vector<NoteLocator>::iterator it = vnl.begin();
    for( ; it != vnl.end(); ++it )
    {
        if( it->getBar() == iBar && it->getIndex() == iIndex )
        { it = vnl.erase( it ); break; }
    }
    if( vnl.empty() ) { eraseParenthesis(rep); }
}
//------------------------------------------------------------------------------
/*Efface l'ornement a l'index iIndex */
void Composition::eraseOrnement( int iIndex )
{
    if( iIndex >= 0 && iIndex < getNumberOfOrnements() )
    { mOrnements.erase( mOrnements.begin() + iIndex ); }
}
//------------------------------------------------------------------------------
/*Efface l'ornement a l'index iIndex */
void Composition::eraseParenthesis( int iIndex )
{
    if( iIndex >= 0 && iIndex < getNumberOfParenthesis() )
    { mParenthesis.erase( mParenthesis.begin() + iIndex ); }
}
//------------------------------------------------------------------------------
/*Efface le stroke iStrokeIndex de la barre iBar*/
void Composition::eraseStroke( int iBar, int iStrokeIndex )
{
    Bar& b = getBar(iBar);
    if( iStrokeIndex >= 0 && iStrokeIndex < getNumberOfStrokesInBar(iBar) )
    { b.mStrokes.erase( b.mStrokes.begin() + iStrokeIndex ); }
}
//------------------------------------------------------------------------------
/*Retourne l'index de ligne qui contient la barre iBarIndex. Si on ne trouve pas
 de ligne, on retourne -1.*/
int Composition::findLine( int iBarIndex ) const
{
    int r = -1;
    for( int i = 0; i < getNumberOfLines(); ++i )
    {
        if( iBarIndex >= 0 && iBarIndex < getNumberOfBars() &&
           iBarIndex >= getLineFirstBar( i ) ) { r = i; }
    }
    return r;
}
//------------------------------------------------------------------------------
/*Retourne l'index du matra qui contient la note iNoteIndex dans la barre iBar.
 Si on ne trouve rien, on retourne -1.*/
int Composition::findMatra( int iBar, int iNoteIndex ) const
{
    int r = -1;
    const Bar& b = getBar(iBar);
    for( int i = 0; i < getNumberOfMatraInBar(iBar); ++i )
    {
        const vector< int >& g = b.mMatras[i];
        for( int j = 0; j < g.size(); ++j )
        {
            if( g[j] == iNoteIndex ){ r = i; break; }
        }
    }
    return r;
}
//------------------------------------------------------------------------------
/*Retourne l'index de l'ornement qui contient la note iNoteIndex dans la barre
 iBar. Si on ne trouve rien, on retourne -1.*/
int Composition::findOrnement( int iBar, int iNoteIndex ) const
{
    int r = -1;
    for( int i = 0; i < getNumberOfOrnements(); ++i )
    {
        const Ornement& o = mOrnements[i];
        for( int j = 0; j < o.mNotes.size(); ++j )
        {
            if( o.mNotes[j].getBar() == iBar && o.mNotes[j].getIndex() == iNoteIndex )
            { r = i; break; }
        }
    }
    return r;
}
//------------------------------------------------------------------------------
int Composition::findParenthesis( int iBar, int iNoteIndex ) const
{
    int r = -1;
    for( int i = 0; i < getNumberOfParenthesis(); ++i )
    {
        const vector<NoteLocator>& vnl = mParenthesis[i].mNotes;
        for( int j = 0; j < (int)vnl.size(); ++j && i == -1 )
        {
            if( vnl[j].getBar() == iBar && vnl[j].getIndex() == iNoteIndex )
            { r = i; }
        }
    }
    return r;
}
//------------------------------------------------------------------------------
/*Retourne l'index du stroke qui contient la note iNoteIndex dans la barre
 iBar. Si on ne trouve rien, on retourne -1.*/
int Composition::findStroke( int iBar, int iNoteIndex ) const
{
    int r = -1;
    const Bar& b = getBar(iBar);
    for( int i = 0; i < getNumberOfStrokesInBar(iBar); ++i )
    {
        for( int j = 0; j < b.mStrokes[i].mSpan.size(); ++j )
        {
            if( b.mStrokes[i].mSpan[j] == iNoteIndex )
            { r = i; break; }
        }
    }
    return r;
}
//------------------------------------------------------------------------------
void Composition::fromBinary( QByteArray iBa )
{
    clear();
    QDataStream ds(&iBa, QIODevice::ReadOnly );
    
    qint32 magincHeader, version, lowestSupportedVersion;
    ds >> magincHeader;
    ds >> version;
    ds >> lowestSupportedVersion;
    
    if( magincHeader != kMagicHeader )
    { addError( "fromBinary: unknow file format." ); }
    
    if( version > kCurrentVersion && kCurrentVersion < lowestSupportedVersion )
    { addError( "fromBinary: File is too new." ); }
    
    if( magincHeader == kMagicHeader &&
       (version <= kCurrentVersion || kCurrentVersion >= lowestSupportedVersion ) )
    {
        ds.setVersion( QDataStream::Qt_5_4 );
        
        //--- title
        ds >> mTitle;
        
        //--- barres
        qint32 numBars;
        ds >> numBars;
        for( int i = 0; i < numBars; ++i )
        {
            addBar();
            //--- notes
            qint32 numNotes;
            ds >> numNotes;
            for( int j = 0; j < numNotes; ++j )
            {
                qint32 v, m, o;
                ds >> v; ds >> o; ds >> m;
                Note n( (noteValue)v, o, (noteModification)m );
                addNote( i, n );
            }
            
            //--- matras
            qint32 numMatras;
            ds >> numMatras;
            for( int j = 0; j < numMatras; ++j )
            {
                qint32 numNotesInMatra;
                ds >> numNotesInMatra;
                vector<int> noteIndices;
                for( int k = 0; k < numNotesInMatra; ++k )
                {
                    qint32 noteIndex;
                    ds >> noteIndex;
                    noteIndices.push_back(noteIndex);
                }
                addMatra(i, noteIndices);
            }
            
            //--- graceNotes
            qint32 numGraceNotes;
            ds >> numGraceNotes;
            for( int j = 0; j < numGraceNotes; ++j )
            {
                qint32 noteIndex;
                ds >> noteIndex;
                addGraceNote(i, noteIndex);
            }
            
            //--- strokes
            qint32 numStrokes;
            ds >> numStrokes;
            for( int j = 0; j < numStrokes; ++j )
            {
                qint32 st, noteIndex, numNotes;
                vector<int> vn;
                ds >> st;
                ds >> numNotes;
                for( int k = 0; k < numNotes; ++k )
                {
                    ds >> noteIndex;
                    vn.push_back(noteIndex);
                }
                addStroke(i, strokeType(st), vn);
            }
            
            if( version > 2 )
            {
                QString s;
                ds >> s;
                setBarText(i, s);
            }
        }
        
        //--- lignes
        qint32 numLines;
        ds >> numLines;
        for( int i = 0; i < numLines; ++i )
        {
            qint32 firstBar;
            QString text;
            ds >> firstBar;
            ds >> text;
            addLine( firstBar, text );
        }
        
        //--- ornements
        qint32 numOrnements;
        ds >> numOrnements;
        for( int i = 0; i < numOrnements; ++i )
        {
            qint32 ot, numNotes, barIndex, noteIndex;
            vector<NoteLocator> vn;
            ds >> ot; //type
            ds >> numNotes;
            for( int j = 0; j < numNotes; ++j )
            {
                ds >> barIndex;
                ds >> noteIndex;
                vn.push_back( NoteLocator( barIndex, noteIndex ) );
            }
            addOrnement((ornementType)ot, vn);
        }
        
        if(version < 5)
        {
            //--- gamme devient la barre 0
            addDescriptionBar("Scale");
            qint32 numNotes;
            ds >> numNotes;
            for( int i = 0; i < numNotes; ++i )
            {
                qint32 v, m, o;
                ds >> v; ds >> o; ds >> m;
                Note n( (noteValue)v, o, (noteModification)m );
                addNote(0, n);
            }
            
            //--- tarab tuning devient la barre 1
            {
                addDescriptionBar("Tarab tuning");
                qint32 numNotes;
                ds >> numNotes;
                for( int i = 0; i < numNotes; ++i )
                {
                    qint32 v, m, o;
                    ds >> v; ds >> o; ds >> m;
                    Note n( (noteValue)v, o, (noteModification)m );
                    addNote(1, n);
                }
            }
        }
        
        //parenthesis
        if( version >= 4 )
        {
            qint32 numParenthesis; ds >> numParenthesis;
            for( int i = 0; i < numParenthesis; ++i )
            {
                //les notelocator de la parenthese
                vector<NoteLocator> vnl;
                qint32 numNotes; ds >> numNotes;
                for( int j = 0; j < numNotes; ++j )
                {
                    qint32 bar, index;
                    ds>>bar; ds>>index;
                    NoteLocator nl( bar, index );
                    vnl.push_back(nl);
                }
                //le nombre de repet
                qint32 numRep; ds >> numRep;
                
                addParenthesis(vnl, numRep);
            }
        }
        
        //description bars
        //les notes des barres de description sont contenues dans mBars
        //Il ne reste qu'a charger la valeur du label.
        if(version >= 5)
        {
            qint32 numDescriptionBars;
            ds >> numDescriptionBars;
            for(int i = 0; i < numDescriptionBars; ++i)
            {
                QString label;
                ds >> label;
                
                //on ne fait pas addDescriptionBar(QString) car
                //cette méthode ajoute une barre, alors que les barres de
                //description ont déjà été ajouté dans mBars lors du chargement
                //des barres. Il ne reste qu'a seter le label
                mDescriptionBarLabels.push_back(label);
            }
        }
        
        if(version >= 6)
        {
            qint8 showLineNumber;
            ds >> showLineNumber;
            mOptions.mShowLineNumber = (bool)showLineNumber;
        }
    }
}
//------------------------------------------------------------------------------
QString Composition::getAndClearLastErrors() const
{
    QString r = mErrors;
    mErrors = QString();
    return r;
}
//------------------------------------------------------------------------------
vector<int> Composition::getBarsInvolvedByOrnement( int iOrn ) const
{
    vector<int> r;
    if( iOrn >= 0 && iOrn < mOrnements.size() )
    {
        const Ornement& o = mOrnements[iOrn];
        for( int i = 0; i < o.mNotes.size(); ++i )
        { r.push_back( o.mNotes[i].getBar() ); }
        r.erase( std::unique( r.begin(), r.end() ), r.end() );
    }
    return r;
}
//------------------------------------------------------------------------------
vector<int> Composition::getBarsInvolvedByParenthesis( int iRep ) const
{
    vector<int> r;
    if( iRep >= 0 && iRep < mParenthesis.size() )
    {
        const Parenthesis& reps = mParenthesis[iRep];
        for( int i = 0; i < reps.mNotes.size(); ++i )
        { r.push_back( reps.mNotes[i].getBar() ); }
        r.erase( std::unique( r.begin(), r.end() ), r.end() );
    }
    return r;
}
//------------------------------------------------------------------------------
const Composition::Bar& Composition::getBar( int iIndex ) const
{
    const Bar* b = 0;
    if( iIndex >= 0 && iIndex < (int)mBars.size() )
    {
        b = &mBars[iIndex];
    }
    else
    {
        QString s;
        s.sprintf( "Bar %d was requested via getBar(), but is not present in the"
                  "data.", iIndex );
        addError( s );
        b = &mDummyBar;
    }
    return *b;
}
//------------------------------------------------------------------------------
Composition::Bar& Composition::getBar( int iIndex )
{
    return const_cast<Bar&>(
                            const_cast<const Composition*>(this)->getBar( iIndex ) );
}
//------------------------------------------------------------------------------
QString Composition::getBarText( int iBar ) const
{ return getBar(iBar).mText; }
//------------------------------------------------------------------------------
QString Composition::getLabelFromDescriptionBar( int iIndex ) const
{
    QString r;
    if( iIndex >= 0 && iIndex < getNumberOfDescriptionBars() )
    { r = mDescriptionBarLabels[iIndex]; }
    return r;
}
//------------------------------------------------------------------------------
int Composition::getLineFirstBar( int iLine ) const
{ return mLines[iLine].mFirstBar; }
//------------------------------------------------------------------------------
QString Composition::getLineText( int iLine ) const
{ return mLines[iLine].mText; }
//------------------------------------------------------------------------------
Note Composition::getNote( int iBar, int iIndex ) const
{
    Note n(nvSa);
    if( iIndex >= 0 && iIndex < getNumberOfNotesInBar( iBar ) )
    { n = getBar( iBar ).mNotes[iIndex]; }
    return n;
}
//------------------------------------------------------------------------------
int Composition::getNoteIndexFromGraceNote( int iBar, int i ) const
{ return getBar(iBar).mGraceNotes[i]; }
//------------------------------------------------------------------------------
int Composition::getNoteIndexFromMatra( int iBar, int iMatra, int i ) const
{ return getBar(iBar).mMatras[iMatra][i]; }
//------------------------------------------------------------------------------
int Composition::getNoteIndexFromStroke( int iBar, int iStroke, int i ) const
{ return getBar(iBar).mStrokes[iStroke].mSpan[i]; }
//------------------------------------------------------------------------------
NoteLocator Composition::getNoteLocatorFromParenthesis( int iO, int i ) const
{ return mParenthesis[iO].mNotes[i]; }
//------------------------------------------------------------------------------
NoteLocator Composition::getNoteLocatorFromOrnement( int iO, int i ) const
{ return mOrnements[iO].mNotes[i]; }
//------------------------------------------------------------------------------
int Composition::getNumberOfBars() const
{ return mBars.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfDescriptionBars() const
{ return mDescriptionBarLabels.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfLines() const
{ return mLines.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfGraceNotesInBar( int iBar ) const
{ return getBar(iBar).mGraceNotes.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfMatraInBar( int iBar ) const
{ return getBar(iBar).mMatras.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfNotesInBar( int iBar ) const
{ return getBar(iBar).mNotes.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfNotesInMatra( int iBar, int iMatra ) const
{ return getBar( iBar ).mMatras[iMatra].size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfNotesInOrnement( int iOrn ) const
{ return mOrnements[iOrn].mNotes.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfNotesInParenthesis( int iRep ) const
{ return mParenthesis[iRep].mNotes.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfNotesInStroke( int iBar, int iStroke ) const
{ return getBar(iBar).mStrokes[iStroke].mSpan.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfOrnements() const
{ return mOrnements.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfParenthesis() const
{ return mParenthesis.size(); }
//------------------------------------------------------------------------------
int Composition::getNumberOfRepetitionsForParenthesis( int iRep ) const
{ return mParenthesis[iRep].mNumber; }
//------------------------------------------------------------------------------
int Composition::getNumberOfStrokesInBar( int iBar ) const
{ return getBar(iBar).mStrokes.size(); }
//------------------------------------------------------------------------------
Composition::Options Composition::getOptions() const
{ return mOptions; }
//------------------------------------------------------------------------------
ornementType Composition::getOrnementType( int iOrn ) const
{ return mOrnements[iOrn].mOrnementType; }
//------------------------------------------------------------------------------
//La barre 0 contient toujours la gamme...
//Il faut que jempeche d'enlever la barre 0...
vector<Note> Composition::getScale() const
{ return getBar( 0 ).mNotes; }
//------------------------------------------------------------------------------
strokeType Composition::getStrokeType( int iBar, int iStroke ) const
{ return getBar(iBar).mStrokes[iStroke].mStrokeType; }
//------------------------------------------------------------------------------
QString Composition::getTitle() const
{ return mTitle; }
//------------------------------------------------------------------------------
bool Composition::hasBarText( int iBar ) const
{ return !getBarText(iBar).isEmpty(); }
//------------------------------------------------------------------------------
bool Composition::hasError() const
{ return !mErrors.isEmpty(); }
//------------------------------------------------------------------------------
bool Composition::hasStroke( int iBar, int iNoteIndex ) const
{ return findStroke( iBar, iNoteIndex) != -1; }
//------------------------------------------------------------------------------
bool Composition::isGraceNote( int iBar, int iNoteIndex) const
{
    const Bar& b = getBar(iBar);
    return ::find( b.mGraceNotes.begin(), b.mGraceNotes.end(), iNoteIndex ) !=
    b.mGraceNotes.end();
}
//------------------------------------------------------------------------------
bool Composition::isNoteInMatra( int iBar, int iNoteIndex ) const
{ return findMatra( iBar, iNoteIndex ) != -1; }
//------------------------------------------------------------------------------
bool Composition::isNoteInOrnement( int iBar, int iNoteIndex ) const
{ return findOrnement( iBar, iNoteIndex) != -1; }
//------------------------------------------------------------------------------
bool Composition::isNoteInParenthesis( int iBar, int iNoteIndex ) const
{ return findParenthesis( iBar, iNoteIndex) != -1 ; }
//------------------------------------------------------------------------------
bool Composition::isStartOfLine( int iBar ) const
{
    bool r = false;
    for( int i = 0; i < getNumberOfLines(); ++i )
    { if( getLineFirstBar(i) == iBar ) { r = true; break; } }
    return r;
}
//------------------------------------------------------------------------------
bool Composition::ornementAppliesToBar( int iOrn, int iBar ) const
{
    bool r = false;
    const Ornement& o = mOrnements[iOrn];
    for( int i = 0; i < o.mNotes.size(); ++i )
    { if( o.mNotes[i].getBar() == iBar ){ r = true; break; } }
    return r;
}
//------------------------------------------------------------------------------
bool Composition::parenthesisAppliesToBar( int iRep, int iBar ) const
{
    bool r = false;
    const Parenthesis& p = mParenthesis[iRep];
    for( int i = 0; i < (int)p.mNotes.size(); ++i )
    { if( p.mNotes[i].getBar() == iBar ){ r = true; break; } }
    return r;
}
//------------------------------------------------------------------------------
void Composition::setBarText( int iBar, QString iText )
{ getBar(iBar).mText = iText; }
//------------------------------------------------------------------------------
void Composition::setLabelForDescriptionBar( int iIndex, QString iLabel )
{
    if( iIndex >= 0 && iIndex < getNumberOfDescriptionBars() )
    { mDescriptionBarLabels[iIndex] = iLabel; }
}
//------------------------------------------------------------------------------
void Composition::setLineText( int iLineIndex, QString iText )
{
    if( iLineIndex >= 0 && iLineIndex < mLines.size() )
    { mLines[iLineIndex].mText = iText; }
}
//------------------------------------------------------------------------------
void Composition::setNote( int iBar, int iIndex, Note iN )
{
    Bar& b = getBar( iBar );
    if( iIndex >= 0 && iIndex < getNumberOfNotesInBar(iBar) )
    { b.mNotes[iIndex] = iN; }
}
//------------------------------------------------------------------------------
void Composition::setNumberOfRepetitionForParenthesis(int iIndex, int iNumRepetitions)
{
    if( iIndex >= 0 && iIndex < getNumberOfParenthesis() )
    { mParenthesis[iIndex].mNumber = iNumRepetitions; }
}
//------------------------------------------------------------------------------
void Composition::setOptions(const Options& iO)
{
    mOptions = iO;
}

////------------------------------------------------------------------------------
//void Composition::setScale( std::vector<Note> iNote )
//{ mScale.mNotes = iNote; }
////------------------------------------------------------------------------------
//void Composition::setTarabTuning( std::vector<Note> iNote )
//{ getBar(dbTarabTuning).mNotes = iNote; }
//------------------------------------------------------------------------------
void Composition::setTitle( QString iTitle )
{ mTitle = iTitle; }
//------------------------------------------------------------------------------
/*Incrémente de iN tous les indices des notes de grace de la barre iBar qui sont
 supérieur à iFromIndex. */
void Composition::shiftGraceNotes( int iN, int iBar, int iFromIndex )
{
    Bar& b = getBar(iBar);
    for( int i = 0; i < b.mGraceNotes.size(); ++i )
    {
        if( b.mGraceNotes[i] > iFromIndex )
        { b.mGraceNotes[i] += iN; }
    }
}
//------------------------------------------------------------------------------
/*voir méthode shiftGraceNotes()*/
void Composition::shiftLines( int iN, int iFromIndex )
{
    for( int i = 0; i < mLines.size(); ++i )
    {
        if( getLineFirstBar(i) > iFromIndex )
        { mLines[i].mFirstBar += iN; }
    }
}
//------------------------------------------------------------------------------
/*voir méthode shiftGraceNotes()*/
void Composition::shiftMatras(int iN, int iBar, int iFromIndex)
{
    Bar& b = getBar(iBar);
    for( int i = 0; i < getNumberOfMatraInBar(iBar); ++i )
    {
        vector<int>& m = b.mMatras[i];
        for( int j = 0; j < m.size(); ++j )
        {
            if( m[j] > iFromIndex )
            { m[j] += iN; }
        }
    }
}
//------------------------------------------------------------------------------
/*Incrémente de iN l'index de barre des ornements qui ont un index de barre
 supérieur à iFromIndex. */
void Composition::shiftOrnements(int iN, int iFromIndex)
{
    for( int i = 0; i < mOrnements.size(); ++i )
    {
        Ornement& o = mOrnements[i];
        for( int j = 0; j < o.mNotes.size(); ++j )
        {
            const NoteLocator nl = o.mNotes[j];
            if( nl.getBar() > iFromIndex )
            { o.mNotes[j] = NoteLocator( nl.getBar() + iN, nl.getIndex() ); }
        }
    }
}
//------------------------------------------------------------------------------
/*Incrémente de iN l'index des notes qui sont supérieur à iFromIndex dans la
 barre iBar*/
void Composition::shiftOrnements(int iN, int iBar, int iFromIndex)
{
    for( int i = 0; i < getNumberOfOrnements(); ++i )
    {
        Ornement& o = mOrnements[i];
        for( int j = 0; j < getNumberOfNotesInOrnement(i); ++j )
        {
            const NoteLocator nl = getNoteLocatorFromOrnement(i, j);
            if( nl.getBar() == iBar && nl.getIndex() > iFromIndex )
            { o.mNotes[j] = NoteLocator( nl.getBar(), nl.getIndex() + iN ); }
        }
    }
}
//------------------------------------------------------------------------------
/*Incrémente de iN l'index de barre des ornements qui ont un index de barre
 supérieur à iFromIndex. */
void Composition::shiftParenthesis(int iN, int iFromIndex)
{
    for( int i = 0; i < getNumberOfParenthesis(); ++i )
    {
        vector<NoteLocator>& vnl = mParenthesis[i].mNotes;
        for( int j = 0; j < getNumberOfNotesInParenthesis(i); ++j )
        {
            const NoteLocator nl = vnl[j];
            if( nl.getBar() > iFromIndex )
            { vnl[j] = NoteLocator( nl.getBar() + iN, nl.getIndex() ); }
        }
    }
}

//------------------------------------------------------------------------------
/*Incrémente de iN l'index des notes qui sont supérieur à iFromIndex dans la
 barre iBar*/
void Composition::shiftParenthesis(int iN, int iBar, int iFromIndex)
{
    for( int i = 0; i < getNumberOfParenthesis(); ++i )
    {
        vector<NoteLocator>& vnl = mParenthesis[i].mNotes;
        for( int j = 0; j < getNumberOfNotesInParenthesis(i); ++j )
        {
            const NoteLocator nl = vnl[j];
            if( nl.getBar() == iBar && nl.getIndex() > iFromIndex )
            { vnl[j] = NoteLocator( nl.getBar(), nl.getIndex() + iN ); }
        }
    }
}
//------------------------------------------------------------------------------
/*voir méthode shiftGraceNotes()*/
void Composition::shiftStrokes( int iN, int iBar, int iFromIndex)
{
    Bar& b = getBar(iBar);
    for( int i = 0; i < b.mStrokes.size(); ++i )
    {
        vector<int>& vs = b.mStrokes[i].mSpan;
        for( int j = 0; j < vs.size(); ++j )
        {
            if( vs[j] > iFromIndex )
            { vs[j] += iN; }
        }
    }
}
//------------------------------------------------------------------------------
QByteArray Composition::toBinary() const
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    
    ds << (qint32)kMagicHeader;
    ds << (qint32)kCurrentVersion;
    ds << (qint32)kLowestSupportedVersion;
    
    ds.setVersion( QDataStream::Qt_5_4 );
    
    //title
    ds << getTitle();
    
    //--- Barres
    ds << (qint32)getNumberOfBars();
    for( int i = 0; i < getNumberOfBars(); ++i )
    {
        //--- Notes
        //combien de notes dans la barre i
        ds << (qint32)getNumberOfNotesInBar( i );
        for( int j = 0; j < getNumberOfNotesInBar( i ); ++j )
        {
            Note n = getNote( i, j );
            ds << (qint32)n.getValue();
            ds << (qint32)n.getOctave();
            ds << (qint32)n.getModification();
        }
        
        //--- Matras
        //combien de matra dans la barre i
        ds << (qint32)getNumberOfMatraInBar( i );
        for( int j = 0; j < getNumberOfMatraInBar( i ); ++j )
        {
            //combien de notes dans le matra j de la barre i
            ds << (qint32)getNumberOfNotesInMatra( i, j );
            for( int k = 0; k < getNumberOfNotesInMatra( i, j ); ++k )
            { ds << (qint32)getNoteIndexFromMatra(i, j, k); }
        }
        
        //--- GraceNotes
        ds << (qint32)getNumberOfGraceNotesInBar(i);
        for( int j = 0; j < getNumberOfGraceNotesInBar( i ); ++j )
        { ds << (qint32)getNoteIndexFromGraceNote(i, j); }
        
        //--- Strokes
        ds << (qint32)getNumberOfStrokesInBar(i);
        for( int j = 0; j < getNumberOfStrokesInBar( i ); ++j )
        {
            ds << (qint32)getStrokeType(i, j);
            //combien de note dans le stroke
            ds << (qint32)getNumberOfNotesInStroke(i, j);
            for( int k = 0; k < getNumberOfNotesInStroke(i, j); ++k)
            { ds << getNoteIndexFromStroke(i, j, k); }
        }
        
        //--- texte
        ds << getBarText(i);
    }// fin des barres
    
    //--- lignes
    ds << (qint32) getNumberOfLines();
    for( int i = 0; i < getNumberOfLines(); ++i )
    {
        ds << (qint32) getLineFirstBar( i );
        ds << getLineText(i);
    }
    
    //--- ornements
    ds << (qint32) getNumberOfOrnements();
    for( int i = 0; i < getNumberOfOrnements(); ++i )
    {
        //le type
        ds << (qint32)getOrnementType(i);
        //les noteLocator
        ds << (qint32)getNumberOfNotesInOrnement(i);
        for( int j = 0; j < getNumberOfNotesInOrnement(i); ++j )
        {
            const NoteLocator& nl = getNoteLocatorFromOrnement(i, j);
            ds << (qint32)nl.getBar();
            ds << (qint32)nl.getIndex();
        }
    }
    
    //--- parenthesis
    ds << (qint32)getNumberOfParenthesis();
    for( int i = 0; i < getNumberOfParenthesis(); ++i)
    {
        //les notes entre parentheses
        ds << (qint32)getNumberOfNotesInParenthesis(i);
        for( int j = 0; j < getNumberOfNotesInParenthesis(i); ++j )
        {
            const NoteLocator& nl = getNoteLocatorFromParenthesis(i, j);
            ds << (qint32)nl.getBar();
            ds << (qint32)nl.getIndex();
        }
        //le nombre de repetition
        ds << (qint32)getNumberOfRepetitionsForParenthesis(i);
    }
    
    //--- description bars
    ds << (qint32)getNumberOfDescriptionBars();
    for(int i = 0; i < getNumberOfDescriptionBars(); ++i)
    {
        ds << getLabelFromDescriptionBar(i);
    }
    
    //--- options
    ds << (qint8)getOptions().mShowLineNumber;
    return ba;
}


