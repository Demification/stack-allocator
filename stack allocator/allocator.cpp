#include  "allocator.h"

#include <thr/xtimec.h>
#include <thr/xthreads.h>
#include <malloc.h> 

namespace alc
{
	void PAUSE()
	{
		xtime delay;
		xtime_get(&delay, 1);

		delay.nsec = delay.nsec + 10000;
		_Thrd_yield();
		_Thrd_sleep(&delay);
	}

	unsigned long __stdcall allocator::thread_func(void* lpParameter)
	{

		parameters *param = static_cast<parameters*>(lpParameter);

		param->ptr = _alloca(param->size);

		if (param->ptr == nullptr)
		{
			MessageBox(0, L"param->ptr == nullptr", L"allocator", 0);
		}

		SuspendThread(param->handle);
		param->ptr = nullptr;
		return 0;
	}

	const void *allocator::init(const size_t blocksize, const size_t blockcount)
	{
		//SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(1) << 0);

		this->blockcount = blockcount;
		this->blocksize = blocksize;

		unsigned int size = blocksize % 16;
		if (size)
		{
			size = 16 - size;
			this->blocksize = blocksize + size;
		}

		param.size = this->blocksize*this->blockcount + sizeof(block) * this->blockcount;

		param.handle = CreateThread(0, param.size + 16, &thread_func, (void*)&param, CREATE_SUSPENDED, 0);
		//SetThreadAffinityMask(param.handle, static_cast<DWORD_PTR>(1) << 0);
		
		ResumeThread(param.handle);

		const size_t max_i = 1024 * 1024;
		size_t i = 0;
		while (!param.ptr)
		{
			PAUSE();
			i++;
			if (i == max_i)
			{
				MessageBox(0, L"i == max_i", L"allocator", 0);
				return nullptr;
			}
		};

		memset(param.ptr, 0, param.size);

		masterinc = 0;

		size_t bytes = this->blockcount * sizeof(block) + this->blockcount * this->blocksize;

		blocks = (block*)param.ptr;
		const BYTE* data = (const BYTE*)((size_t)param.ptr + this->blockcount * sizeof(block));

		block* b0 = (block*)blocks;
		for (size_t i = 0; i < this->blockcount; i++)
		{
			b0->ptr = (void*)(&data[i*this->blocksize]);
			++b0;
		}

		sizebytes = this->blockcount * this->blocksize;
		return (void*)blocks[0].ptr;
	};

	const void *allocator::init(const size_t bytes)
	{
		return init(bytes, 1);
	}

	void allocator::destroy()
	{
		if (param.handle)
		{
			ResumeThread(param.handle);

			while (param.ptr)
			{
				PAUSE();
			};

			CloseHandle(param.handle);
			param.handle = nullptr;
		}
	};

	allocator::block* allocator::allocate()
	{
		b0 = (block*)blocks;
		for (size_t i = 0; i < blockcount; i++)
		{
			if (b0->master == 0)
			{
				masterinc++;
				b0->master = masterinc;
				return b0;
			}

			++b0;
		}
		return nullptr;
	};

	allocator::block* allocator::allocate(const size_t block_count)
	{
		if (block_count == 1)
		{
			return allocate();
		}
		else if (block_count > 1)
		{
			b0 = (block*)blocks;
			int master0 = 0;
			size_t j = 0;

			for (size_t i = 1; i <= this->blockcount; i++)
			{

				if (b0->master == 0)
				{
					master0++;
				}
				else
				{
					master0 = 0;
					++b0;
					continue;
				}

				if (master0 == block_count)
				{
					b1 = b0;
					masterinc++;
					for (j = i; j > i - block_count; j--)
					{
						b1->master = masterinc;
						--b1;
					}
					++b1;
					return b1;
				}
				++b0;
			}
		}
		return nullptr;
	};

	void allocator::free(block* pBlock)
	{
		if (!param.handle) return;
		size_t i = 0;
		do
		{
			i = pBlock->master;
			pBlock->master = 0;
			++pBlock;
		} while (pBlock->master == i);
	};

	void allocator::allfree()
	{
		for (size_t i = 0; i < blockcount; i++)
		{
			((block*)blocks)[i].master = 0;
		}
	};

	const allocator::block* allocator::getblock(const size_t index)
	{
		return &((block*)blocks)[index];
	};

	void* allocator::malloc(size_t bytes)
	{
		size_t i = 0;
		if (bytes%blocksize != 0) i++;
		size_t j = i + (size_t)(bytes / blocksize);

		return (void*)allocate(j)->ptr;
	};

	void allocator::free(const void* ptr)
	{
		for (size_t i = 0; i < blockcount; i++)
		{
			if (blocks[i].ptr == ptr)
			{
				free(&blocks[i]);
				return;
			}
		}
	};

	void* allocator::get0ptr()
	{
		return (void*)getblock(0)->ptr;
	}

}