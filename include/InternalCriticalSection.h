#ifndef INTERNALCRITICALSECTION_H
#define INTERNALCRITICALSECTION_H

#include <pthread.h>
#include "Commons.h"


namespace SynchroPrimitives
{
  typedef struct CriticalSection
  {
    pthread_mutex_t mutex;
    CriticalSection();
    ~CriticalSection();
  } CriticalSection, *LPMCriticalSection;

  void EnterInternalCriticalSection(LPMCriticalSection lpMcs);
  void LeaveInternalCriticalSection(LPMCriticalSection lpMcs);
}

#endif // INTERNALCRITICALSECTION_H
