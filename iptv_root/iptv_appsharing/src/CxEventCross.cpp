///////////////////////////////////////////////////////////////////////////////
// CxEvent.cpp
// -----------
// Event
//
// Author : Guilherme Cox <cox@ieee.org>
// Updated: Mon Aug 15 18:00:43 BRT 2005
///////////////////////////////////////////////////////////////////////////////
#include "iptv_shared/shared/compat.h"
#include "CxEventCross.h"
///////////////////////////////////////////////////////////////////////////////
CxEvent::CxEvent( bool bInitialState ) : m_Mutex(SEMAPH_MUTEX)
{
	if (bInitialState)
		m_Mutex.Wait();
}

BOOL CxEvent::SetEvent()
{
    return m_Mutex.Signal();
}

BOOL CxEvent::Wait()
{
	return m_Mutex.Wait();
}

///////////////////////////////////////////////////////////////////////////////

