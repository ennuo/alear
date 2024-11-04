#ifndef MEM_STL_BUCKETS_H
#define MEM_STL_BUCKETS_H

#include <limits>

#include "mem_allocator.h"

template <class T> class STLBucketAlloc;
template <> class STLBucketAlloc<void>
{
public:
    typedef void              value_type;
    typedef value_type*       pointer;
    typedef value_type const* const_pointer;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;

    template <class U>
    struct rebind
    {
        typedef STLBucketAlloc<U> other;
    };
};

template <class T>
class STLBucketAlloc
{
public:
    typedef T                 value_type;
    typedef value_type&       reference;
    typedef value_type const& const_reference;
    typedef value_type*       pointer;
    typedef value_type const* const_pointer;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;

    template <class U>
    struct rebind
    {
        typedef STLBucketAlloc<U> other;
    };

    STLBucketAlloc() throw() {}  // not required, unless used
    template <class U> STLBucketAlloc(STLBucketAlloc<U> const& u) throw() {}

    pointer allocate(size_type n, STLBucketAlloc<void>::const_pointer = 0)
    {
        return static_cast<pointer>(CAllocatorMM::Malloc(gStlBucket, n*sizeof(value_type)));
    }

    void deallocate(pointer p, size_type)
    {
        CAllocatorMM::Free(gStlBucket, p);
    }

    void construct(pointer p, value_type const& val)
    {
        ::new(p) value_type(val);
    }

    void destroy(pointer p)
    {
        p->~value_type();
    }

    size_type max_size() const throw()
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }

    pointer address(reference x) const
    {
        return &x;
    }

    const_pointer address(const_reference x) const
    {
        return &x;
    }
};

#endif // MEM_STL_BUCKETS_H