#include "common.h"
#include "system.h"

static struct ptrmarker m_PtrMarkers[1024*8];
static int m_NumPtrMarkers;

void addMarker(u32 ptr_src, uintptr_t ptr_host)
{
	if (m_NumPtrMarkers >= ARRAYCOUNT(m_PtrMarkers)) {
		sysLogPrintf(LOG_ERROR, "Marker limit exceeded");
		exit(EXIT_FAILURE);
	}

	m_PtrMarkers[m_NumPtrMarkers].ptr_src = ptr_src;
	m_PtrMarkers[m_NumPtrMarkers].ptr_host = ptr_host;
	m_NumPtrMarkers++;
}

struct ptrmarker* findPtrMarker(uintptr_t ptr_src)
{
	for (int i = 0; i < m_NumPtrMarkers; i++) {
		if (m_PtrMarkers[i].ptr_src == ptr_src) {
			return &m_PtrMarkers[i];
		}
	}

	return NULL;
}

void resetMarkers()
{
	m_NumPtrMarkers = 0;
}
