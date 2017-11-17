
#ifndef realisim_utils_timer_hh
#define realisim_utils_timer_hh

#include <chrono>

namespace realisim
{
namespace utils
{
  //Cette classe fonctionne comme un chronomètre. Elle sert a calculer
  //des intervals de temps.
  
  class Timer
  {
  public:
    Timer();
    virtual ~Timer();
    
    double getElapsed() const;
//void pause();
    void start();
    void stop();
    
  protected:
    enum state{ sIdle, sRunning, sStopped };
    
    Timer(const Timer&);
    Timer& operator=(const Timer&);
    
    state getState() const {return mState;}
    void goToState(state);
    
    state mState;
    std::chrono::high_resolution_clock::time_point mT1;
    std::chrono::high_resolution_clock::time_point mT2;
  };

} // end of namespace utils
} // end of namespace realisim
#endif
