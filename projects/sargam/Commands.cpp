
#include "Commands.h"
#include "PartitionViewer.h"

using namespace realisim;
using namespace sargam;

//------------------------------------------------------------------
//--- PartitionViewerCommand
//------------------------------------------------------------------
PartitionViewerCommand::PartitionViewerCommand(PartitionViewer* iP) :
p(iP)
{
  mCompositionState = p->x->toBinary();
  mCurrentCursorPosition = p->getCursorPosition();
  mCurrentSelection = p->mSelectedNotes;
}
//------------------------------------------------------------------
void PartitionViewerCommand::undo()
{
  restoreComposition();
  restoreUi();
}
//------------------------------------------------------------------
void PartitionViewerCommand::restoreComposition()
{
  //On recharge la composition à partir du binaire sauvegardé. La
  //passe est d'appeler setComposition sur le pointeur de composition
  //ainsi, le ui sera correctement rafraichit comme si on avait
  //ouvert une nouvelle composition.
  p->x->fromBinary(mCompositionState);
  p->resetComposition(p->x);
}
//------------------------------------------------------------------
void PartitionViewerCommand::restoreUi()
{
  p->setCursorPosition(mCurrentCursorPosition);
  p->mSelectedNotes = mCurrentSelection;
}

//Les commandes définies avec la macor MAKE_COMMAND_CLASS peuvent
//être implémentée avec la macro suivantes.
#define MAKE_COMMAND_IMPLEMENTATION(COMMAND_NAME) \
COMMAND_NAME::COMMAND_NAME( PartitionViewer* iP ) : \
PartitionViewerCommand(iP), \
p(iP) \
{} \
\
void COMMAND_NAME::execute() \
{ \
restoreUi(); \
p->do ## COMMAND_NAME(); \
} \

//------------------------------------------------------------------
//--- CommandAddBar
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandAddBar);
//------------------------------------------------------------------
//--- CommandAddDescriptionBar
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandAddDescriptionBar);
//------------------------------------------------------------------
//--- CommandAddGraceNotes
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandAddGraceNotes);
//------------------------------------------------------------------
//--- CommandAddLine
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandAddLine);
//------------------------------------------------------------------
//--- CommandAddMatra
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandAddMatra);
//------------------------------------------------------------------
//--- CommandAddNote
//------------------------------------------------------------------
CommandAddNote::CommandAddNote( PartitionViewer* iP, Note iN ) :
PartitionViewerCommand(iP),
p(iP),
n(iN)
{}
//------------------------------------------------------------------
void CommandAddNote::execute()
{
  restoreUi();
  p->doCommandAddNote(n);
}
//------------------------------------------------------------------
//--- CommandAddOrnement
//------------------------------------------------------------------
CommandAddOrnement::CommandAddOrnement( PartitionViewer* iP,
                                       ornementType iO ) :
PartitionViewerCommand(iP),
p(iP),
o(iO)
{}
//------------------------------------------------------------------
void CommandAddOrnement::execute()
{
  restoreUi();
  p->doCommandAddOrnement(o);
}
//------------------------------------------------------------------
//--- CommandAddParenthesis
//------------------------------------------------------------------
CommandAddParenthesis::CommandAddParenthesis( PartitionViewer* iP,
                                       int iNumber ) :
PartitionViewerCommand(iP),
p(iP),
n(iNumber)
{}
//------------------------------------------------------------------
void CommandAddParenthesis::execute()
{
  restoreUi();
  p->doCommandAddParenthesis(n);
}
//------------------------------------------------------------------
//--- CommandAddStroke
//------------------------------------------------------------------
CommandAddStroke::CommandAddStroke( PartitionViewer* iP,
                                    strokeType iSt ) :
PartitionViewerCommand(iP),
p(iP),
s(iSt)
{}
//------------------------------------------------------------------
void CommandAddStroke::execute()
{
  restoreUi();
  p->doCommandAddStroke(s);
}
//------------------------------------------------------------------
//--- CommandBreakMatrasFromSelection
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandBreakMatrasFromSelection);
//------------------------------------------------------------------
//--- CommandBreakOrnementsFromSelection
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandBreakOrnementsFromSelection);
//------------------------------------------------------------------
//--- CommandDecreaseOctave
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandDecreaseOctave);
//------------------------------------------------------------------
//--- CommandErase
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandErase);
//------------------------------------------------------------------
//--- CommandIncreaseOctave
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandIncreaseOctave);
//------------------------------------------------------------------
//--- CommandRemoveDescriptionBar
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandRemoveDescriptionBar);
//------------------------------------------------------------------
//--- CommandRemoveParenthesis
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandRemoveParenthesis);
//------------------------------------------------------------------
//--- CommandRemoveParenthesis
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandRemoveSelectionFromGraceNotes);
//------------------------------------------------------------------
//--- CommandRemoveStroke
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandRemoveStroke);
//------------------------------------------------------------------
//--- CommandShiftNote
//------------------------------------------------------------------
MAKE_COMMAND_IMPLEMENTATION(CommandShiftNote);



