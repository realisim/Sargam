#include <algorithm>
#include <cmath>
#include "Statistics.h"
#include <limits>


using namespace realisim;
using namespace utils;

Statistics::Statistics() :
  mKeepSamples(false),
  mNumberOfSamples(0),
  mSum(0.0),
  mSumSquared(0.0),
  mMin(std::numeric_limits<double>::max()),
  mMax(-std::numeric_limits<double>::max())
{}

//-------------------------------------------------------------------------
Statistics::Statistics(const Statistics& iRhs) :
mKeepSamples(iRhs.isKeepingSamples()),
mNumberOfSamples(iRhs.getNumberOfSamples()),
mSum(iRhs.mSum),
mSumSquared(iRhs.mSumSquared),
mMin(iRhs.mMin),
mMax(iRhs.mMax)
{}

//-------------------------------------------------------------------------
Statistics::~Statistics()
{ clear(); }

//-------------------------------------------------------------------------
void Statistics::add(double iSample)
{
  if(isKeepingSamples())
  { mSamples.push_back(iSample); }
  
  ++mNumberOfSamples;
  mSum += iSample;
  mSumSquared += iSample * iSample;
  mMin = std::min(mMin, iSample);
  mMax = std::max(mMax, iSample);
}

//-------------------------------------------------------------------------
void Statistics::add(const std::vector<double>& iSamples)
{
  for(size_t i = 0; i < iSamples.size(); ++i )
  { add(iSamples[i]); }
}

//-------------------------------------------------------------------------
void Statistics::add(const double* iSamples, unsigned int iNum)
{
  for(size_t i = 0; i < iNum; ++i )
  { add( iSamples[i] ); }
}
  
//-------------------------------------------------------------------------
void Statistics::clear()
{
  mSamples.clear();
  mNumberOfSamples = 0;
  mSum = 0.0;
  mSumSquared = 0.0;
  mMin = std::numeric_limits<double>::max();
  mMax = -std::numeric_limits<double>::max();
}

//-------------------------------------------------------------------------
double Statistics::getMean() const
{
  return mSum / getNumberOfSamples();
}

//-------------------------------------------------------------------------
double Statistics::getSample(unsigned int iIndex) const
{
  double r = 0.0;
  if(iIndex < getNumberOfSamples())
  { r = mSamples[iIndex]; }
  return r;
}

//-------------------------------------------------------------------------
double Statistics::getStandardDeviation() const
{
  const unsigned int n = getNumberOfSamples();
  return sqrt( (mSumSquared - ( mSum * mSum / n)) / (n - 1) );
}

