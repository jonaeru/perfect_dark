#include "common.h"
#include "system.h"

struct ptrmarker m_PtrMarkers[2048];
int m_NumPtrMarkers;

void add_marker(u32 ptr_src, uintptr_t ptr_host)
{
	if (m_NumPtrMarkers >= ARRAYCOUNT(m_PtrMarkers)) {
		sysLogPrintf(LOG_ERROR, "[BG] Marker limit exceeded");
		exit(EXIT_FAILURE);
	}

	m_PtrMarkers[m_NumPtrMarkers].ptr_src = ptr_src;
	m_PtrMarkers[m_NumPtrMarkers].ptr_host = ptr_host;
	m_NumPtrMarkers++;
}

struct ptrmarker* find_marker(uintptr_t ptr_src)
{
	for (int i = 0; i < m_NumPtrMarkers; i++) {
		if (m_PtrMarkers[i].ptr_src == ptr_src) {
			return &m_PtrMarkers[i];
		}
	}

	return NULL;
}

void reset_markers()
{
	m_NumPtrMarkers = 0;
}
