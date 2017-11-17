
#ifndef realisim_utils_statistics_hh
#define realisim_utils_statistics_hh

#include <vector>

namespace realisim
{
namespace utils
{

  //Ce bout de code est fortement inspiré de :
  //http://c.learncodethehardway.org/book/ex43.html
  
  //Cette classe sert a compiler des statistiques sur un flux d'échantillons.
  //La classe permet optionellement de conserver les échantillons, sinon,
  //les statisques sont calculés à la demande en temps constant O(1).
  //
  //On peut obtenir,
  // min
  // max
  // mean
  // nombre d'échantillons
  // écart type
  // 
  class Statistics
  {
  public:
    Statistics();
    Statistics(const Statistics&);
    virtual ~Statistics();
    
    void add(double);
    void add(const std::vector<double>&);
    void add(const double*, unsigned int);
	void clear();
    double getMax() const {return mMax;}
    double getMean() const;
    double getMin() const {return mMin;}
    unsigned int getNumberOfSamples() const {return mNumberOfSamples;}
    double getSample(unsigned int) const;
    double getStandardDeviation() const;
    bool isKeepingSamples() const {return mKeepSamples;}
    void keepSamples(bool iK) {mKeepSamples = iK;}
    
  protected:
    Statistics& operator=(const Statistics&);    
    
    bool mKeepSamples;
    std::vector<double> mSamples;
    unsigned int mNumberOfSamples;
    double mSum;
    double mSumSquared;
    double mMin;
    double mMax;
  };

} // end of namespace utils
} // end of namespace realisim
#endif
