#pragma once
#include <windows.h>
#include <malloc.h>

namespace alc
{
	__forceinline static void PAUSE();

	class allocator
	{
	private:
		size_t masterinc;

		static unsigned long __stdcall thread_func(void* lpParameter);

	public:
		size_t sizebytes;

		struct parameters
		{
			void* ptr = nullptr;
			size_t size;
			HANDLE handle;
		} param;

		struct block
		{
			const void *ptr;
			size_t master;
		};

		const void *init(const size_t blocksize, const size_t blockcount);

		const void *init(const size_t bytes);

		void destroy();

		block* allocate();

		block* allocate(const size_t block_count);

		void free(block* pBlock);

		void allfree();

		const block* getblock(const size_t index);

		void* malloc(size_t bytes);

		void free(const void* ptr);

		void* get0ptr();

	protected:
		block *b0, *b1;
		const block *blocks;
		size_t blockcount;
		size_t blocksize;
	};

}
