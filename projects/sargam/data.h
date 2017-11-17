//
//  data.h

#ifndef realisim_sargam_data_h
#define realisim_sargam_data_h

#include <QString>
#include <vector>

namespace realisim
{
namespace sargam
{
    
    enum noteValue{ nvSa = 1, nvRe, nvGa, nvMa, nvPa, nvDha, nvNi, nvComma, nvChik,
        nvRest };
    enum ornementType{ otMeend, otKrintan, otAndolan, otGamak };
    enum noteModification{ nmKomal, nmShuddh, nmTivra };
    enum strokeType{ stDa, stRa, stDiri, stNone };
    enum script{ sLatin = 0, sDevanagari, sNumberOfScript };
    
    //------------------------------------------------------------------------------
    struct Version
    {
        Version() : mMajor(0), mMinor(0), mRevision(0) {}
        Version(int iMajor, int iMinor, int iRev) : mMajor(iMajor), mMinor(iMinor), mRevision(iRev) {}
        Version(const Version&) = default;
        Version& operator=(const Version&) = default;
        ~Version() = default;
        
        bool operator<(const Version& iV)
        {
            return mMajor < iV.mMajor ||
            (mMajor <= iV.mMajor && mMinor < iV.mMinor) ||
            (mMajor <= iV.mMajor && mMinor <= iV.mMinor && mRevision < iV.mRevision );
        }
        
        int mMajor;
        int mMinor;
        int mRevision;
    };
    
    //------------------------------------------------------------------------------
    class Note
    {
    public:
        Note( noteValue );
        Note( noteValue, int, noteModification = nmShuddh);
        
        bool canBeKomal() const; //a mettre dans un utilitaire?
        bool canBeTivra() const;
        noteModification getModification() const;
        int getOctave() const;
        noteValue getValue() const;
        void setModification( noteModification );
        void setOctave( int );
        void setValue( noteValue );
        
    protected:
        noteValue mValue;
        int mOctave;
        noteModification mModification;
    };
    //------------------------------------------------------------------------------
    struct Stroke
    {
        Stroke( strokeType, int );
        Stroke( strokeType, std::vector<int> );
        
        strokeType mStrokeType;
        std::vector<int> mSpan;
    };
    //------------------------------------------------------------------------------
    class NoteLocator
    {
    public:
        NoteLocator();
        NoteLocator( int, int );
        bool operator<( const NoteLocator& ) const;
        bool operator<=( const NoteLocator& ) const;
        bool operator>( const NoteLocator& ) const;
        bool operator>=( const NoteLocator& ) const;
        bool operator==( const NoteLocator& ) const;
        bool operator!=( const NoteLocator& ) const;
        
        int getBar() const;
        int getIndex() const;
        static NoteLocator invalid();
        bool isValid() const;
        
    protected:
        int mBar;
        int mIndex;
    };
    
    //------------------------------------------------------------------------------
    /*
     
     Notes:
     On suppose que les notes ne seront pas dans plus de 1 matras/ornement/stroke
     simultanément.
     
     Explication sur les barres et barres de description
     */
    class Composition
    {
    public:
        Composition();
        Composition(const Composition&) = default;
        Composition& operator=(const Composition&) = default;
        ~Composition() = default;
        
        struct Options
        {
            Options();
            Options(const Options&) = default;
            Options& operator=(const Options&) = default;
            ~Options() = default;
            
            bool mShowLineNumber;
        };
        
        void addBar();
        void addBar( int );
        void addDescriptionBar(QString);
        void addLine( int, QString = QString() );
        void addMatra( int, std::vector<int> );
        void addNote( int, Note );
        void addNote( int, int, Note );
        void addOrnement( ornementType, std::vector<NoteLocator> );
        void addParenthesis( std::vector<NoteLocator>, int = 2 );
        void addStroke( int, strokeType, int );
        void addStroke( int, strokeType, std::vector<int> );
        void addGraceNote( int, int );
        void clear();
        void eraseBar( int );
        bool eraseDescriptionBar( int );
        void eraseLine( int );
        void eraseGraceNote( int, int );
        void eraseMatra( int, int );
        void eraseNote( int, int );
        void eraseOrnement( int );
        void eraseParenthesis( int );
        void eraseStroke( int, int );
        int findLine( int ) const;
        int findMatra( int, int ) const;
        int findOrnement( int, int ) const;
        int findParenthesis( int, int ) const;
        int findStroke( int, int ) const;
        void fromBinary( QByteArray );
        QString getAndClearLastErrors() const;
        QString getBarText( int ) const;
        std::vector<int> getBarsInvolvedByOrnement( int ) const;
        std::vector<int> getBarsInvolvedByParenthesis( int ) const;
        QString getLabelFromDescriptionBar( int ) const;
        int getLineFirstBar( int ) const;
        QString getLineText( int ) const;
        Note getNote( int, int ) const;
        int getNoteIndexFromGraceNote( int iBar, int i ) const;
        int getNoteIndexFromMatra( int iBar, int iMatra, int i ) const;
        int getNoteIndexFromStroke( int iBar, int iStroke, int i ) const;
        NoteLocator getNoteLocatorFromOrnement( int iO, int i ) const;
        NoteLocator getNoteLocatorFromParenthesis( int iO, int i ) const;
        int getNumberOfBars() const;
        int getNumberOfDescriptionBars() const;
        int getNumberOfLines() const;
        int getNumberOfGraceNotesInBar( int ) const;
        int getNumberOfMatraInBar( int ) const;
        int getNumberOfNotesInBar( int ) const;
        int getNumberOfNotesInMatra( int, int ) const;
        int getNumberOfNotesInOrnement( int ) const;
        int getNumberOfNotesInParenthesis( int ) const;
        int getNumberOfNotesInStroke( int, int) const;
        int getNumberOfOrnements() const;
        int getNumberOfParenthesis() const;
        int getNumberOfRepetitionsForParenthesis( int ) const;
        int getNumberOfStrokesInBar( int ) const;
        Options getOptions() const;
        ornementType getOrnementType( int ) const;
        std::vector<Note> getScale() const;  //a effacer?
        strokeType getStrokeType( int iBar, int i ) const;
        QString getTitle() const;
        bool hasError() const;
        bool hasBarText( int ) const;
        bool hasStroke( int, int ) const;
        bool isGraceNote( int, int) const;
        bool isNoteInMatra( int, int ) const;
        bool isNoteInOrnement( int, int ) const;
        bool isNoteInParenthesis(int, int) const;
        bool isStartOfLine( int ) const;
        bool ornementAppliesToBar( int, int ) const;
        bool parenthesisAppliesToBar( int, int ) const;
        void setBarText( int, QString );
        void setLabelForDescriptionBar( int, QString );
        void setLineText( int, QString );
        void setNote( int, int, Note );
        void setNumberOfRepetitionForParenthesis(int, int);
        void setOptions(const Options&);
        void setTitle( QString );
        QByteArray toBinary() const;
        
    protected:
        struct Bar
        {
            std::vector<Note> mNotes;
            std::vector< std::vector<int> > mMatras;
            std::vector<int> mGraceNotes;
            std::vector<Stroke> mStrokes;
            QString mText;
        };
        
        struct Ornement
        {
            Ornement();
            Ornement( ornementType, std::vector<NoteLocator> );
            
            ornementType mOrnementType;
            std::vector<NoteLocator> mNotes;
        };
        
        struct Line
        {
            Line();
            Line( int, QString );
            bool operator<( const Line& ) const;
            
            int mFirstBar;
            QString mText;
        };
        
        struct Parenthesis
        {
            Parenthesis();
            Parenthesis( std::vector<NoteLocator>, int );
            
            std::vector<NoteLocator> mNotes;
            int mNumber;
        };
        
        void addNoteToMatra( int, int, int );
        void addNoteToOrnement( int, int, int );
        void addNoteToParenthesis( int, int, int );
        void addError( QString ) const;
        std::vector<Note> defaultScale() const;
        std::vector<Note> defaultTarabTuning() const;
        void eraseNoteFromMatra( int, int );
        void eraseNoteFromOrnement( int, int );
        void eraseNoteFromParenthesis( int, int );
        
        const Bar& getBar(int) const;
        Bar& getBar( int );
        Note& getNoteForModification( int, int );
        void shiftGraceNotes( int, int, int );
        void shiftLines( int, int );
        void shiftMatras(int, int, int);
        void shiftOrnements(int, int);
        void shiftOrnements(int, int, int);
        void shiftParenthesis(int, int);
        void shiftParenthesis(int, int, int);
        void shiftStrokes( int, int, int );
        
        //-- data
        static Bar mDummyBar;
        static Note mDummyNote;
        QString mTitle;
        std::vector<Bar> mBars;
        std::vector< QString > mDescriptionBarLabels;
        std::vector<Line> mLines;
        std::vector<Ornement> mOrnements;
        std::vector< Parenthesis > mParenthesis;
        Options mOptions;
        mutable QString mErrors;
    };
    
}
}

#endif
