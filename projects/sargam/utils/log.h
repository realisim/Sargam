/*
 *  log.h
 *  Pierre-Olivier Beaudoin on 21 sept 2014.
 */
 
#ifndef realisim_utils_log_hh
#define realisim_utils_log_hh

#include <QFile>
#include <QTextStream>
#include <vector>

/* Cette classe sert a logger dans la console ou dans un fichier. Le format 
  d'entré est le même que printf.

	Les options console ou fichier se font à partir des méthodes 
  logToConsole(bool) et logToFile(bool). Chaque entrée via la méthode
  log( char*, ... ) peut être préfixée d'un timestamp si l'option logTimestamp()
  est active.
  
  Les méthodes logsToConsole(), logsToFile et logsTimestamp servent à interroger
  les options correspondantes.
  
  Le fichier doit être spécifié par setLogPath( QString ) ou par
  logToFile( bool, QString ). Le path peut être un nom de fichier, un chemin 
  relatif ou un chemin absolu. Si le chemin n'est pas absolu, il sera par
  rapport au chemin absolu d'où l'application s'exécute.
  
  La méthode takeEntriesFrom( Log& ), sert a prendre et à logger les entrées
  d'une autre Log. La log passé en paramètre sera vider via la méthode clear().
*/
namespace realisim
{
namespace utils 
{

//ajouter le necessaire pour mode verbose...

class Log
{
public:
	Log();
  virtual ~Log();
  
  virtual void clear();
	virtual QString getLogPath() const;
  virtual int getNumberOfEntries() const;
  virtual QString getEntry( int ) const;
//bool isVerbose() const;
  void log( const char *, ... ) const; //support printf
	virtual void logToConsole( bool );
  virtual void logTimestamp( bool );
  virtual void logToFile( bool, QString = "" );
  virtual bool logsToConsole() const;
  virtual bool logsToFile() const;
  virtual bool logsTimestamp() const;
//virtual void setAsVervbose(bool);
  virtual void setLogPath( QString );
  virtual void takeEntriesFrom( Log& );
   
protected:
	Log( const Log& ); //pas de constructeur copie
  Log& operator=( const Log& ); //pas doperateur egal
	void doLog( QString ) const;  

	mutable std::vector<QString> mEntries;
  QFile mFile;
  mutable QTextStream mStream;
  bool mLogsToConsole;
  bool mLogsToFile;
  bool mLogsTimestamp;
};

}
}

#endif