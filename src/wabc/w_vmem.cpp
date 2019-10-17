#include "w_vmem.h"

namespace wabc
{
#define PAGEHDR(p) ((vmem::pagehdr*)(void *)(size_t(p) & (~size_t(vmem::alloc_granularity - 1))))
#define NODE(p) ((vmem::node_t *)(p))

	// --------------------------------------------------------------------

	static size_t free_index(size_t bytes_to_alloc)
	{
		const size_t nSize = 16;
		size_t first = 0, n = nSize, m, tmp;

		bytes_to_alloc = (bytes_to_alloc + vmem::alloc_unit - 1) / vmem::alloc_unit;
		bytes_to_alloc *= vmem::alloc_unit;

		assert(bytes_to_alloc >= 16);
		while (0 < n)
		{
			m = n / 2;
			tmp = first + m;

			const size_t bytes_of_alloc = 1 << tmp;

			if (bytes_of_alloc < bytes_to_alloc)
			{
				first = tmp + 1;
				n -= m + 1;
			}
			else if (bytes_to_alloc < bytes_of_alloc)
				n = m;
			else
			{
				assert(tmp>3);
				return tmp - 3;
			}
		}
		assert(first > 3);
		return first - 3;
	}

	// --------------------------------------------------------------------

	static void set_alloc(void *p, bool b)
	{
		vmem::pagehdr &hdr = *PAGEHDR(p);
		const size_t index = (size_t(p) - size_t(&hdr)) / vmem::alloc_unit;
		assert(((size_t(p) - size_t(&hdr)) % vmem::alloc_unit) == 0);
		assert(index / 8 < countof(hdr.is_alloc));

		const unsigned char mask = 1 << (index % 8);
		unsigned char &ch = hdr.is_alloc[index / 8];
		if (b)
			ch |= mask;
		else
			ch &= ~mask;
	}

	// --------------------------------------------------------------------

	static unsigned char is_alloc(void *p)
	{
		vmem::pagehdr &hdr = *PAGEHDR(p);
		const size_t index = (size_t(p) - size_t(&hdr)) / vmem::alloc_unit;
		assert(((size_t(p) - size_t(&hdr)) % vmem::alloc_unit) == 0);
		assert(index / 8 < countof(hdr.is_alloc));

		const unsigned char mask = 1 << (index % 8);
		const unsigned char &ch = hdr.is_alloc[index / 8];
		return ch & mask;
	}

	// --------------------------------------------------------------------

	static void set_alloc_size(void *p, size_t flag)
	{
		assert(flag <= 0x0F);

		vmem::pagehdr &hdr = *PAGEHDR(p);
		const size_t index = (size_t(p) - size_t(&hdr)) / vmem::alloc_unit;
		assert(((size_t(p) - size_t(&hdr)) % vmem::alloc_unit) == 0);
		assert(index / 2 < countof(hdr.alloc_flag));
		unsigned char &ch = hdr.alloc_flag[index / 2];

		// 4bit一项
		if (index % 2)
		{
			ch &= 0x0F;
			ch |= ((unsigned char)flag) << 4;
		}
		else
		{
			ch &= 0xF0;
			ch |= ((unsigned char)flag);
		}
	}

	// --------------------------------------------------------------------

	static size_t get_alloc_size(void *p)
	{
		const vmem::pagehdr &hdr = *PAGEHDR(p);

		const size_t index = (size_t(p) - size_t(&hdr)) / vmem::alloc_unit;
		assert(((size_t(p) - size_t(&hdr)) % vmem::alloc_unit) == 0);
		assert(index / 2 < countof(hdr.alloc_flag));
		unsigned char ch = hdr.alloc_flag[index / 2];

		// 4bit一项
		if (index % 2)
			ch >>= 4;
		else
			ch &= 0x0F;
		return ch;
	}
	
	// --------------------------------------------------------------------
	// vmem
	// --------------------------------------------------------------------

	vmem::vmem(size_t fProtect) :m_protect_flag(fProtect)
	{
		SYSTEM_INFO a = { 0 };
		::GetSystemInfo(&a);
		if (a.dwAllocationGranularity % alloc_granularity)
		{
			::wprintf(L"The allocation granularity is not 64k!\n");
			exit(-10);
		}

		for (size_t i = 0; i < countof(m_free_nodes); ++i)
		{
			node_t &a = m_free_nodes[i];
			a.next = a.prior = &a;
		}
	}

	// --------------------------------------------------------------------

	vmem::~vmem()
	{
		node_t *p;
		while (m_free_nodes[0].next != m_free_nodes)
		{
			p = m_free_nodes[0].next;
			m_free_nodes[0].next = p->next;
			::VirtualFree(p, 0, MEM_RELEASE);
		}
	}

	// --------------------------------------------------------------------

	void vmem::clear()
	{
		node_t *p;
		while (m_free_nodes[0].next != m_free_nodes)
		{
			p = m_free_nodes[0].next;
			m_free_nodes[0].next = p->next;
			::VirtualFree(p, 0, MEM_RELEASE);
		}

		for (size_t i = 0; i < countof(m_free_nodes); ++i)
		{
			node_t &a = m_free_nodes[i];
			a.next = a.prior = &a;
		}
	}

	// --------------------------------------------------------------------

	void * vmem::alloc(size_t bytes_to_alloc)
	{
		if (bytes_to_alloc > 32 * 1024)
		{
			void *p = (void *)::VirtualAlloc(0, bytes_to_alloc, MEM_RESERVE, (DWORD)m_protect_flag);
			if (p)
				p = ::VirtualAlloc(p, bytes_to_alloc, MEM_COMMIT, (DWORD)m_protect_flag);

			assert((size_t(p) % alloc_granularity) == 0);
			return p;
		}
		else
		{
			const size_t index = free_index(bytes_to_alloc);
			size_t i;
			while (1)
			{
			__again:
				if (m_free_nodes[index].next != m_free_nodes + index)
				{
					node_t *p = m_free_nodes[index].next;
					p->unlink();
					set_alloc_size(p, index + 3);
					set_alloc(p, true);

					PAGEHDR(p)->bytes_of_alloc += size_t(1) << (index + 3);
					return p;
				}

				for (i = index + 1; i < countof(m_free_nodes); ++i)
				{
					if (m_free_nodes[i].next != m_free_nodes + i)
					{
						node_t *p = m_free_nodes[i].next;
						while (i>index)
							split(p, i--);
						goto __again;
					}
				}
				new_page(bytes_to_alloc);
			}
			return 0;
		}
	}

	// --------------------------------------------------------------------

	void vmem::free(void *p)
	{
		if (size_t(p) & (alloc_granularity - 1))
		{
			pagehdr *pH = PAGEHDR(p);
			const size_t f = get_alloc_size(p);
			assert(f >= 4 && f < 16);
			assert(is_alloc(p));
			NODE(p)->link(m_free_nodes + f - 3);
			set_alloc(p, false);
			merge(p, f);

			pH->bytes_of_alloc -= 1 << f;
			assert(pH->bytes_of_alloc >= 0);
			if (pH->bytes_of_alloc == 0)
			{
#ifdef _DEBUG
				for (size_t i = 1; i < countof(m_free_nodes);++i)
				{
					if (m_free_nodes[i].next != m_free_nodes+i)
					{
						int ii= 0;
					}
				}
#endif
				// 先释放第一页的0.5k和1k
				assert(is_alloc(pH->buf + sizeof(pagehdr)) == 0);
				assert(is_alloc(pH->buf + sizeof(pagehdr)+512) == 0);

				NODE(pH->buf + sizeof(pagehdr))->unlink();
				NODE(pH->buf + sizeof(pagehdr)+512)->unlink();

				// 断开链表
				const size_t last = pH->commit_page_count*page_size;
				size_t first = page_size;

				while (first < last)
				{
					// 必须保证first是在2的j次方的地址上
					size_t i = alloc_granularity, j = page_count;
					const size_t n = last - first;
					for (; i>0 && i > n; i /= 2, --j);

					for (; i > 0; i /= 2, --j)
					{
						if ((first &(i - 1)) == 0)
						{
							assert(j > 3);
							assert(is_alloc(pH->buf + first) == 0);
							NODE(pH->buf + first)->unlink();
							first += i;
							break;
						}
					}
					assert(i);
				}

				assert(first == last);

				pH->node.unlink();
				::VirtualFree(pH, 0, MEM_RELEASE);
			}
		}
		else
		{
			assert((size_t(p) &(alloc_granularity - 1)) == 0);
			::VirtualFree(p, 0, MEM_RELEASE);
		}
	}

	// --------------------------------------------------------------------

	void * vmem::realloc(void *p, size_t new_size)
	{
		if (p)
		{
			size_t bytes_of_copy;
			if (size_t(p) & (alloc_granularity - 1))
			{
				const size_t f = get_alloc_size(p);
				assert(f >= 4 && f < 16);
				if (new_size <= (size_t(1) << f))
					return p;
				bytes_of_copy = 1 << f;
			}
			else
			{
				MEMORY_BASIC_INFORMATION mbi = { 0 };
				::VirtualQuery(p, &mbi, sizeof(mbi));
				if (mbi.RegionSize >= new_size)
					return p;
				bytes_of_copy = mbi.RegionSize;
			}

			void *new_p = alloc(new_size);
			::memcpy(new_p, p, bytes_of_copy);
			this->free(p);
			return new_p;
		}
		else
			return alloc(new_size);
	}

	// --------------------------------------------------------------------

	void * vmem::realloc(void *p, size_t new_size, size_t old_size)
	{
		if (p)
		{
			if (new_size > old_size)
			{
				if (size_t(p) & (alloc_granularity - 1))
				{
					const size_t f = get_alloc_size(p);
					assert(f >= 4 && f < 16);
					if (new_size <= (size_t(1) << f))
						return p;
				}
				else
				{
					MEMORY_BASIC_INFORMATION mbi = { 0 };
					::VirtualQuery(p, &mbi, sizeof(mbi));
					if (mbi.RegionSize >= new_size)
						return p;
				}

				void *new_p = alloc(new_size);
				::memcpy(new_p, p, old_size);
				this->free(p);

				return new_p;
			}
			else
				return p;
		}
		else
			return alloc(new_size);
	}

	// --------------------------------------------------------------------

	void vmem::split(node_t *left, size_t free_node_index)
	{
		assert(free_node_index>1 && free_node_index <countof(m_free_nodes));
		--free_node_index;

		node_t *right = NODE(size_t(left) + (size_t(1) << (free_node_index + 3)));
		left->unlink();

		// left child
		left->link(m_free_nodes + free_node_index);

		// right child
		right->link(m_free_nodes + free_node_index);

		// 分裂后，这两块是独立的可用块
		free_node_index += 3;
		set_alloc_size(left, free_node_index);
		set_alloc_size(right, free_node_index);
	}

	// --------------------------------------------------------------------

	unsigned char * vmem::merge(void *p, size_t alloc_flag)
	{
		pagehdr *pH = PAGEHDR(p);
		node_t *left = (node_t *)p, *right;
		assert(alloc_flag > 3);

		while ((alloc_flag + 1) < 16)
		{
			// 确保内存地址在相应的边界上
			assert((size_t(left) & ((size_t(1) << alloc_flag) - 1)) == 0);

			// 确保不合并页头
			assert((void *)left != pH);

			const size_t n = (size_t(left) - size_t(pH)) >> alloc_flag;
			const size_t is_right_child = n % 2;

			if (is_right_child)
			{
				void * p = (void *)(size_t(left) - (size_t(1) << alloc_flag));

				// 若不为0，正在被使用
				if (p == pH || is_alloc(p) || get_alloc_size(p) != alloc_flag)
					break;
				right = left;
				left = (node_t *)p;
			}
			else
			{
				right = (node_t *)(size_t(left) + (size_t(1) << alloc_flag));

				// 若不为0，正在被使用
				if (int(size_t(right) - size_t(pH)) >= (pH->commit_page_count * page_size) || is_alloc(right) || alloc_flag != get_alloc_size(right))
					break;
			}

			left->unlink();
			right->unlink();

			alloc_flag++;
			NODE(left)->link(m_free_nodes + alloc_flag - 3);

			// 右孩子不能其它块合并了
			set_alloc_size(right, 0);
		}

		set_alloc_size(left, alloc_flag);
		return (unsigned char *)(left) + (1 << alloc_flag);
	}

	// --------------------------------------------------------------------

	void vmem::new_page(size_t bytes_to_alloc)
	{
		const size_t count_of_new_page = (bytes_to_alloc + page_size - 1) / page_size;
		assert(count_of_new_page < page_count);
		size_t n;
		if (m_free_nodes[0].next != m_free_nodes)
		{
			pagehdr &hdr = *PAGEHDR(m_free_nodes[0].prior);
			if (hdr.commit_page_count < page_count)
			{
				// 还有其它页面没提交
				n = page_count - hdr.commit_page_count;
				if (n > count_of_new_page)
					n = count_of_new_page;

				n *= page_size;
				unsigned char *p = (unsigned char *)::VirtualAlloc(hdr.buf + hdr.commit_page_count*page_size, n,
					MEM_COMMIT, (DWORD)m_protect_flag), *first, *last;
				assert(p);

				first = p;
				last = p + n;

				// 链接到4k页链表
				for (; first < last; first += page_size)
				{
					NODE(first)->link(m_free_nodes + 9);
					set_alloc_size(first, 12);
				}

				// 尝试合并
				for (first = p; first < last;)
					first = merge(first, 12);

				hdr.commit_page_count += n / page_size;

				assert(first == last);
				return;
			}
		}

		// allocate new one page
		pagehdr *pH = (pagehdr *)::VirtualAlloc(0, alloc_granularity, MEM_RESERVE, (DWORD)m_protect_flag);
		pH = (pagehdr *)::VirtualAlloc(pH, page_size, MEM_COMMIT, (DWORD)m_protect_flag);

		assert((size_t(pH) & (alloc_granularity - 1)) == 0);

		// 清0
		::memset(pH, 0, sizeof(*pH));

		pH->node.link(m_free_nodes); // 添加到页表链里
		pH->commit_page_count = 1;

		// 现在有1.5k的空闲空间
		__wabc_static_assert(sizeof(*pH) == 2048 + 512);
		// 0.5k
		NODE(pH->buf + sizeof(pagehdr))->link(m_free_nodes + 6);
		set_alloc_size(pH->buf + sizeof(pagehdr), 6);
		// 1k
		NODE(pH->buf + sizeof(pagehdr)+512)->link(m_free_nodes + 7);
		set_alloc_size(pH->buf + sizeof(pagehdr)+512, 7);
	}
}