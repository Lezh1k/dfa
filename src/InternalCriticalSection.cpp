#include <unistd.h>
#include "InternalCriticalSection.h"

void SynchroPrimitives::EnterInternalCriticalSection(LPMCriticalSection lpMcs ) {
  pthread_mutex_lock(&lpMcs->mutex);
}
//////////////////////////////////////////////////////////////////////////

void SynchroPrimitives::LeaveInternalCriticalSection(LPMCriticalSection lpMcs ) {
  pthread_mutex_unlock(&lpMcs->mutex);
}
//////////////////////////////////////////////////////////////////////////

/*init critical section :)*/
SynchroPrimitives::CriticalSection::CriticalSection() {
  pthread_mutex_init(&mutex, NULL);
}

SynchroPrimitives::CriticalSection::~CriticalSection() {
  pthread_mutex_destroy(&mutex);
}
//////////////////////////////////////////////////////////////////////////
