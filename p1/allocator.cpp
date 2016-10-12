#include "allocator.h"
#include <algorithm>
#include <iostream>


Allocator::Allocator(void *base, size_t s) :
		base_ptr(base), size(s), bitmask(std::vector<bool>(s, false)), ptrs(
				std::vector<std::shared_ptr<Pointer>>())
{}

Pointer Allocator::alloc(size_t N)
{
	bool is_enough_mem = false;
	size_t curr_len = 0;
	size_t offset = 0;
	for (size_t i = 0; i < size; ++i)
	{
		if (bitmask[i] == false)
		{
			curr_len++;
		}
		else
		{
			offset = i + 1;
			curr_len = 0;
		}

		if (curr_len == N)
		{
			is_enough_mem = true;
			break;
		}
	}

	if (!is_enough_mem)
	{
		throw AllocError(AllocErrorType::NoMemory, "Not enough memory!\n");
	}

	size_t end = offset + N;
	for (size_t i = offset; i < end; i++)
	{
		bitmask[i] = true;
	}
	void **start = new void *(getBase() + offset);
	ptrs.push_back(std::shared_ptr<Pointer>(new Pointer(start, N)));
	return *ptrs.back();
}

void Allocator::realloc(Pointer &p, size_t N)
{
	if (p.getSize() == 0)
	{
		p = alloc(N);
		return;
	}

	size_t offset = p.getPtr() - getBase();
	if (p.getSize() > N)
	{
		for (size_t i = offset + N; i < offset + p.getSize(); i++)
		{
			bitmask[i] = false;
		}
		return;
	}
	bool is_extendable = true;
	for (size_t i = offset + p.getSize(); i < offset + N; i++)
	{
		if (bitmask[i] == true)
		{
			is_extendable = false;
			break;
		}
	}
	if (is_extendable)
	{
		for (size_t i = offset + p.getSize(); i < offset + N; i++)
		{
			bitmask[i] = true;
		}
		p.setSize(N);
	}
	else
	{
		Pointer new_p = alloc(N);
		std::memcpy(new_p.get(), p.get(), p.getSize());
		free(p);
		p = new_p;
	}
}

void Allocator::free(Pointer &p)
{
	size_t offset = p.getPtr() - (char *)base_ptr;
	size_t end = offset + p.getSize();
	int ind = -1;
	for (int i = 0; i < ptrs.size(); i++)
	{
		if (ptrs[i]->get() == p.get() && ptrs[i]->getSize() == p.getSize())
		{
			ind = i;
			break;
		}
	}
	if (ind == -1)
	{
		throw AllocError(AllocErrorType::InvalidFree, "Invalid Free!\n");
	}

	for (size_t i = offset; i < end + 1; i++)
	{
		bitmask[i] = false;
	}
	ptrs.erase(ptrs.begin() + ind);
	p = Pointer();
}

bool compare(std::shared_ptr<Pointer> a, std::shared_ptr<Pointer> b)
{
	return a->get() < b->get();
}

void Allocator::defrag()
{
	size_t length = 0;
	char *move_to = getBase();
	sort(ptrs.begin(), ptrs.end(), compare);

	for (std::shared_ptr<Pointer> i : ptrs)
	{
		char *ptr_beg = i->getPtr();
		char *ptr_end = ptr_beg + i->getSize();
		size_t ptr_size = i->getSize();
		memmove(move_to, ptr_beg, ptr_size);
		i->setPtr(move_to);
//		new_ptrs.push_back(i);

		move_to += ptr_size;
		length += ptr_size;
	}

	for (size_t i = 0; i < length; i++)
	{
		bitmask[i] = true;
	}
	for (size_t i = length; i < size; i++)
	{
		bitmask[i] = false;
	}
}

