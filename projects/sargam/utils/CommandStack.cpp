/*
 *  CommandStack.cpp
 *  Created by Pierre-Olivier Beaudoin on 09-08-05.
 */

#include "utils/Command.h"
#include "utils/CommandStack.h"

using namespace std;
using namespace realisim;
  using namespace utils;
  
CommandStack::CommandStack() :
  mCommands(),
  mIndex(-1)
{}

CommandStack::~CommandStack()
{ clear(); }

//------------------------------------------------------------------------------
/*Ajoute une commande a la fin de la pile. CommandStack devient le propriétaire
du pointeur: i.e. Le pointeur sera deleter à la destruction ou lorsqu'une 
nouvelle commande est insérée et que l'index de la pile n'est pas à la fin
de la pile. De plus, toutes les commandes qui étaient après l'index sont
détruites.*/
void CommandStack::add(Command* ipC)
{
  //si l'index n'est pas positionné à la fin de la pile
  if(mIndex < (int)mCommands.size() - 1)
  {
    //ont détruit toutes le commandes qui suivent l'index
    for(int i = mIndex + 1; i < (int)mCommands.size(); ++i)
      delete mCommands[i];
    
      //les parentheses sur mIndex + 1 sont importants sous windows... sans elles,
      //ca plante!
      mCommands.erase(mCommands.begin() + (mIndex + 1), mCommands.end());
  }
  
  //on ajoute et execute la commande insérée
  mCommands.push_back(ipC);
  mIndex = mCommands.size() - 1;
}

//------------------------------------------------------------------------------
void CommandStack::clear()
{
  for(unsigned int i = 0; i < mCommands.size(); ++i)
    delete mCommands[i];
  mCommands.clear();
}

//------------------------------------------------------------------------------
int CommandStack::getCurrentCommandIndex() const
{ return mIndex; }

//------------------------------------------------------------------------------
int CommandStack::getNumberOfCommands() const
{ return (int)mCommands.size(); }

//------------------------------------------------------------------------------
bool CommandStack::isEmpty() const
{ return getNumberOfCommands() == 0; }

//------------------------------------------------------------------------------
/*ré-exécute la prochaine commande de la pile et place l'index sur cette 
commande*/
void CommandStack::redo()
{
  if(mIndex < (int)mCommands.size() - 1)
    mCommands[++mIndex]->redo();
}

//------------------------------------------------------------------------------
//dé-exécute la commande actuelle et désincrémente l'index de la pile
void CommandStack::undo()
{
  if(mIndex >= 0)
   mCommands[mIndex--]->undo();
}


