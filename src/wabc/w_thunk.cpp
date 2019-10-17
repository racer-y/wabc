#include "w_thunk.h"
#include "w_vmem.h"

namespace wabc
{
	// --------------------------------------------------------------------

	static vmem & instance()
	{
		static vmem g_thunk_vmem(PAGE_EXECUTE_READWRITE);
		return g_thunk_vmem;
	}

	// --------------------------------------------------------------------

	void * thunk_alloc(size_t new_size)
	{
		return instance().alloc(new_size);
	}

	// --------------------------------------------------------------------

	void * thunk_realloc(void *p, size_t new_size)
	{
		return instance().realloc(p, new_size);
	}

	// --------------------------------------------------------------------

	void thunk_free(void *p)
	{
		instance().free(p);
	}
}