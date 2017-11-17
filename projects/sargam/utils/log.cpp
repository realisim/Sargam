/*
 *  CommandStack.cpp
*  Pierre-Olivier Beaudoin on 21 sept 2014.
 */

#include <QDateTime>
#include "utils/log.h"
#include <stdarg.h>

using namespace std;
using namespace realisim;
  using namespace utils;

Log::Log() : 
mEntries(),
mFile(), 
mStream(),
mLogsToConsole( true ),
mLogsToFile( false ),
mLogsTimestamp( true )
{}

Log::~Log()
{ clear(); }
//-----------------------------------------------------------------------------
void Log::clear()
{	
	mEntries.clear();
  mStream.setDevice( 0 );
  mFile.close();
}
//-----------------------------------------------------------------------------
void Log::doLog( QString iS ) const
{
	QString toLog;
	if( logsTimestamp() )
  {
  	toLog += QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss.zzz" ) 
      + ": ";
  }
  toLog += iS;
	mEntries.push_back( toLog );
  if( logsToConsole() )
  { printf( "%s\n", toLog.toStdString().c_str() ); }
  if( logsToFile() && mStream.device() )
  { mStream << toLog << "\n"; mStream.flush(); }
}
//-----------------------------------------------------------------------------
QString Log::getLogPath() const
{ return mFile.fileName(); }
//-----------------------------------------------------------------------------
int Log::getNumberOfEntries() const
{ return mEntries.size(); }
//-----------------------------------------------------------------------------
QString Log::getEntry(int i) const
{ return mEntries[i]; }
//-----------------------------------------------------------------------------
void Log::log( const char *ipFormat, ... ) const //support printf
{
	va_list args;
  va_start( args, ipFormat );
  int n = vsnprintf( 0, 0, ipFormat, args);
  va_end( args );
	
  va_start( args, ipFormat );
  char* b = new char[n+1]; //+1 pour le \0
  vsprintf( b, ipFormat, args );
  QString s(b);  
  doLog( s );
  delete[] b;
  va_end( args );
}
//-----------------------------------------------------------------------------
void Log::logToConsole(bool iL)
{ mLogsToConsole = iL; }
//-----------------------------------------------------------------------------
void Log::logToFile(bool iL, QString iPath /*= ""*/)
{
	mLogsToFile = iL;
  if( !iPath.isEmpty() ) 
  {
  	setLogPath( iPath );
    logToFile( true );
  }
}
//-----------------------------------------------------------------------------
void Log::logTimestamp(bool iL)
{ mLogsTimestamp = iL; }
//-----------------------------------------------------------------------------
bool Log::logsToConsole() const
{ return mLogsToConsole; }
//-----------------------------------------------------------------------------
bool Log::logsToFile() const
{ return mLogsToFile; }
//-----------------------------------------------------------------------------
bool Log::logsTimestamp() const
{ return mLogsTimestamp; }
//-----------------------------------------------------------------------------
void Log::setLogPath( QString iPath )
{
	mStream.setDevice(0);
  mFile.close();
  mFile.setFileName( iPath );
  if( mFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  { mStream.setDevice( &mFile ); }
}
//-----------------------------------------------------------------------------
void Log::takeEntriesFrom( Log& iLog )
{
	bool wasLoggingTimestamp = logsTimestamp();
  if( iLog.logsTimestamp() ){ logTimestamp(false); }
	for( int i = 0; i < iLog.getNumberOfEntries(); ++i )
  { doLog( iLog.getEntry( i ) ); }
  iLog.clear();
  logTimestamp(wasLoggingTimestamp);
}

