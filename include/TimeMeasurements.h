#ifdef TIME_MEASUREMENTS
#ifndef TIMEMEASUREMENTS_H
#define TIMEMEASUREMENTS_H

#include <chrono>
#include <map>
#include <stdio.h>
#include "IFunctor.h"

enum MeasurementType {
  MT_construct_dfa_table = 0,
  MT_srm_dma_load_table,
};

enum AverageMeasurementTypes {
  AMT_dma_write = MT_srm_dma_load_table+1,
  AMT_dma_read,
  AMT_file_read,
  AMT_file_seek,
  AMT_network_transfer
};

static const char* MeasurementTypeStr[] = {
  "construct_dfa_table",
  "srm_dma_load_table",
  "dma_write",
  "dma_read",
  "file_read",
  "file_seek",
  "network_transfer"
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class CTimeMeasurements
{
private:
  CTimeMeasurements() {}                                  // Private constructor
  ~CTimeMeasurements() {}
  CTimeMeasurements(const CTimeMeasurements&);                 // Prevent copy-construction
  CTimeMeasurements& operator=(const CTimeMeasurements&);      // Prevent assignment

  struct measurement {
    double value;
    size_t count;
    measurement() : value(0.0), count(0){}
  };

  typedef std::map<int, measurement> dct_measurement;
  dct_measurement m_dct_one_shot_measurements;
  std::map<int, dct_measurement> m_dct_average_measurements;

  std::chrono::duration<double> measure_functor(IFunctor* func) {
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();
    (*func)();
    end = std::chrono::high_resolution_clock::now();
    return end-start;
  }

public:

  static CTimeMeasurements& Instance(void) {
    static CTimeMeasurements instance;
    return instance;
  }
  ////////////////////////////////////////////////////////////////////////////

  void one_shot_measure(MeasurementType mt, IFunctor* func) {
    std::chrono::duration<double> time = measure_functor(func);
    m_dct_one_shot_measurements[mt].value = time.count();
    ++m_dct_one_shot_measurements[mt].count;
  }
  ////////////////////////////////////////////////////////////////////////////

  void average_measure(int id, AverageMeasurementTypes amt, IFunctor* func) {
    std::chrono::duration<double> time = measure_functor(func);
    if (m_dct_average_measurements[id].find(amt) == m_dct_average_measurements[id].end()) {
      m_dct_average_measurements[id][amt].value = time.count();
      ++m_dct_average_measurements[id][amt].count;
      return;
    }
    m_dct_average_measurements[id][amt].value += time.count();
    ++m_dct_average_measurements[id][amt].count;
  }
  ////////////////////////////////////////////////////////////////////////////

  void print_results(void) const {
    printf("\nMeasurements result:\n");
    printf("Name\t\tValue\t\tCount\n");
    for (auto i = m_dct_one_shot_measurements.begin();
         i != m_dct_one_shot_measurements.end(); ++i) {
      printf("%s\t%lf\t%lu\n", MeasurementTypeStr[i->first], i->second.value, i->second.count);
    }
    printf("\nFile\tName\t\tValue\t\tCount\t\tTime\n");
    for (auto i = m_dct_average_measurements.begin();
         i != m_dct_average_measurements.end(); ++i) {
      for (auto j = i->second.begin(); j != i->second.end(); ++j) {
        printf("%d\t%s\t%lf\t%lu\t%lf\n",
               i->first, MeasurementTypeStr[j->first],
            j->second.value / j->second.count, j->second.count, j->second.value);
      }
    }
  }
  ////////////////////////////////////////////////////////////////////////////

  const dct_measurement& dct_one_shot_measurements(void) const {return m_dct_one_shot_measurements;}
  const std::map<int, dct_measurement>& dct_average_measurements(void) const {return m_dct_average_measurements;}
};

#endif // TIMEMEASUREMENTS_H
#endif // TIME_MEASUREMENTS
