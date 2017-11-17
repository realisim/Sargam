
#ifndef Realisim_sargam_Commands_h
#define Realisim_sargam_Commands_h

#include "utils/Command.h"
#include "data.h"
#include <vector>

namespace realisim
{
namespace sargam
{
class PartitionViewer;

class PartitionViewerCommand : public utils::Command
{
public:
  PartitionViewerCommand(PartitionViewer*);
  virtual ~PartitionViewerCommand(){}
  
  virtual void execute(){;}
  virtual void undo() override;
  
protected:
  void restoreComposition();
  void restoreUi();
  
  PartitionViewer* p;
  QByteArray mCompositionState;
  NoteLocator mCurrentCursorPosition;
  std::vector< std::pair<int, int> > mCurrentSelection;
};

//Afin de simplifier lajout de commandes, la macro suivante peut
//être utilisée. Plusieurs commandes sont identiques.
#define MAKE_COMMAND_CLASS(COMMAND_NAME) \
class COMMAND_NAME : public PartitionViewerCommand \
{ \
public: \
COMMAND_NAME( PartitionViewer* ); \
virtual ~COMMAND_NAME() {} \
virtual void execute() override; \
protected: \
PartitionViewer* p; \
}; \

//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandAddBar);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandAddDescriptionBar);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandAddGraceNotes);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandAddLine);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandAddMatra);
//--------------------------------------------------------------------
class CommandAddNote : public PartitionViewerCommand
{
public:
  CommandAddNote( PartitionViewer*, Note );
  virtual ~CommandAddNote() {}
  virtual void execute() override;
protected:
  PartitionViewer* p;
  Note n;
};
//--------------------------------------------------------------------
class CommandAddOrnement : public PartitionViewerCommand
{
public:
  CommandAddOrnement( PartitionViewer*, ornementType );
  virtual ~CommandAddOrnement() {}
  virtual void execute() override;
protected:
  PartitionViewer* p;
  ornementType o;
};
//--------------------------------------------------------------------
class CommandAddParenthesis : public PartitionViewerCommand
{
public:
  CommandAddParenthesis( PartitionViewer*, int );
  virtual ~CommandAddParenthesis() {}
  virtual void execute() override;
protected:
  PartitionViewer* p;
  int n;
};
//--------------------------------------------------------------------
class CommandAddStroke : public PartitionViewerCommand
{
public:
  CommandAddStroke( PartitionViewer*, strokeType );
  virtual ~CommandAddStroke() {}
  virtual void execute() override;
protected:
  PartitionViewer* p;
  strokeType s;
};
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandBreakMatrasFromSelection);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandBreakOrnementsFromSelection);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandDecreaseOctave);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandErase);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandIncreaseOctave);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandRemoveDescriptionBar);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandRemoveParenthesis);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandRemoveSelectionFromGraceNotes);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandRemoveStroke);
//--------------------------------------------------------------------
MAKE_COMMAND_CLASS(CommandShiftNote);

  
  
}
}


#endif
