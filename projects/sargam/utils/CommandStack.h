/*
 *  CommandStack.h
 *  Created by Pierre-Olivier Beaudoin on 09-08-05.
 */
 
#ifndef realisim_utils_commandManager_hh
#define realisim_utils_commandManager_hh

namespace realisim {namespace utils {class Command;}} 
#include <vector>

/*Cette classe gère les besoins de undo/redo. Il suffit de sous classer 
realisim::utils::Command afin de pouvoir ajouter les commandes et le 
CommandManager gère la file de commandes.

Chaque nouvelle commande ajoutée est placée à la fin de la file. Lorsque l'index
n'est pas a la fin de la pile (un ou plusieurs undo ont été effectués) la 
prochaine commande sera insérée après l'index et toutes les commandes qui
suivaient l'index sont effacées.

mCommands: la liste des commandes
mIndex: identifie l'emplacement actuel dans la pile de commandes.
*/
namespace realisim
{
namespace utils 
{
	class CommandStack
 	{
  	public:
    CommandStack();
    virtual ~CommandStack();
     
    void add(Command*);
    int getCurrentCommandIndex() const;
    int getNumberOfCommands() const;
    bool isEmpty() const;
    void clear();
    void redo();
    void undo();
     
  protected:       
    std::vector<Command*> mCommands;
    int mIndex;
  };
}
}

#endif