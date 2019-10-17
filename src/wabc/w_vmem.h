/*
* Copyright[yyyy][name of copyright owner]
* Copyright (c) 2019, Yuan zhi wei <Racer_y@126.com>. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http ://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include "w_std.h"

namespace wabc
{
	class vmem
	{
	public:
		enum{
			alloc_granularity = 64 * 1024, alloc_unit = 16,
			page_size = 4 * 1024, page_count = alloc_granularity / page_size,
		};

		// --------------------------------------------------------------------

		struct node_t
		{
			node_t *next, *prior;

			void link(node_t *h)
			{
				next = h;
				prior = h->prior;

				h->prior->next = this;
				h->prior = this;
			}

			void unlink()
			{
				prior->next = next;
				next->prior = prior;
			}
		};

		// --------------------------------------------------------------------
		// 第一页的2.5k空间用作管理
		struct pagehdr
		{
			union
			{
				union
				{
					struct
					{
						node_t node;	// 必须在头
						int bytes_of_alloc;		// 已经分配的字节数
						int commit_page_count;	// 总共16页，bit?为1，表明页面已经提交
					};
					char dummy[64];
				};
				unsigned char alloc_flag[2 * 1024]; // 4位一项。若为0，表明未分配
				unsigned char buf[1];
			};
			unsigned char is_alloc[512]; // 0:未分配，1：分配
		};

	public:
		explicit vmem(size_t fProtect = PAGE_READWRITE);
		~vmem();

		void * alloc(size_t bytes_to_alloc);
		void * realloc(void *p, size_t new_size);
		void free(void *p);

		template<typename T>
		T * alloc()
		{
			return (T*)alloc(sizeof(T));
		}

		void * realloc(void *p, size_t new_size, size_t old_size);
		void clear();
	private:
		const size_t m_protect_flag;	// PAGE_READWRITE
		// 0: head 1:16 2:32 3:64 4:128 5:256 6:512 7:1k 8:2k 9:4k 10:8k 11:16k 12:32k
		node_t m_free_nodes[13];

		void split(node_t *left, size_t free_node_index);

		// 返回合并后右边的边界
		unsigned char * merge(void *p, size_t alloc_flag);
		void new_page(size_t bytes_to_alloc);
	};
}