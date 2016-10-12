#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <memory>

enum class AllocErrorType
{
	InvalidFree, NoMemory,
};

class AllocError: std::runtime_error
{
private:
	AllocErrorType type;

public:
	AllocError(AllocErrorType _type, std::string message) :
			runtime_error(message), type(_type)
	{
	}

	AllocErrorType getType() const
	{
		return type;
	}
};

class Allocator;

class Pointer
{
//	void *ptr;
//	size_t size;
	std::shared_ptr<void *> ptr;
	std::shared_ptr<size_t> size;
public:
	Pointer() : ptr(new void*(nullptr)), size(new size_t(0)) {}
	Pointer(void **p, size_t s) : ptr(p), size(new size_t(s)) {}
	void *get() const { return *ptr; }
	size_t getSize() { return *size; }
	char *getPtr() { return (char *) get(); }
	void setSize(size_t new_size) { *size = new_size; }
	void setPtr(void *p) { *ptr = p; }
};

class Allocator
{
	void *base_ptr;
	size_t size;
	std::vector<bool> bitmask;
//	std::vector<Pointer *> ptrs;
	std::vector<std::shared_ptr<Pointer>> ptrs;
public:
	char *getBase() { return (char *) base_ptr; }

	Allocator(void *base, size_t size);

	Pointer alloc(size_t N);
	void realloc(Pointer &p, size_t N);
	void free(Pointer &p);

	void defrag();
	std::string dump() { return ""; }
};

