#pragma once
///@file

#include "nixexpr.hh"
#include "symbol-table.hh"

#include <algorithm>

#include "attr-cache.hh"

namespace nix {

class EvalState;
struct Value;

static_assert(sizeof(Attr) == 2 * sizeof(uint32_t) + sizeof(Value *),
    "performance of the evaluator is highly sensitive to the size of Attr. "
    "avoid introducing any padding into Attr if at all possible, and do not "
    "introduce new fields that need not be present for almost every instance.");

/**
 * Bindings contains all the attributes of an attribute set. It is defined
 * by its size and its capacity, the capacity being the number of Attr
 * elements allocated after this structure, while the size corresponds to
 * the number of elements already inserted in this structure.
 */
class Bindings
{
public:
    typedef uint32_t size_t;
    PosIdx pos;

private:
    // local capacity management
    size_t size_, capacity_;
    // cache management
    bigAttrCache& attrDiskCache_;
    size_t range_start_;

    Bindings(size_t capacity, bigAttrCache& attrDiskCache) : size_(0), capacity_(capacity), attrDiskCache_(attrDiskCache)
    {
        // Allocate enough space in the vector cache for our items.
        this->attrDiskCache_.reserve(capacity);
        range_start_ = this->attrDiskCache_.size();
        for (size_t i = 0; i < capacity; i++)
        {
            this->attrDiskCache_.push_back(Attr{});
        }
    }
    Bindings(const Bindings & bindings) = delete;

public:
    size_t size() const { return size_; }

    bool empty() const { return !size_; }

    typedef bigAttrCache::iterator iterator;

    void push_back(const Attr & attr)
    {
        assert(size_ < capacity_);
        attrDiskCache_[range_start_ + size_] = attr;
        size_++;
    }

    iterator find(Symbol name)
    {
        Attr key(name, 0);
        iterator i = std::lower_bound(begin(), end(), key);
        if (i != end() && i->name == name)
        {
            return i;
        }
        return end();
    }

    std::unique_ptr<Attr> get(Symbol name)
    {
        Attr key(name, 0);
        iterator i = std::lower_bound(begin(), end(), key);
        if (i != end() && i->name == name)
        {
            auto ret = std::unique_ptr<Attr>(new Attr(i->name, i->value, i->pos));
            return ret;
        }
        return nullptr;
    }

    iterator begin()
    {
        return boost::make_iterator_range(attrDiskCache_.begin() + range_start_, attrDiskCache_.begin() + range_start_ + size_).begin();
    }
    iterator end()
    {
        return boost::make_iterator_range(attrDiskCache_.begin() + range_start_, attrDiskCache_.begin() + range_start_ + size_).end();
    }

    Attr operator[](size_t pos)
    {
        auto a = attrDiskCache_[range_start_ + pos];
        return Attr(a.name, a.value, a.pos) ;
    }

    void sort();

    size_t capacity() { return capacity_; }

    /**
     * Returns the attributes in lexicographically sorted order.
     */
    std::vector<std::unique_ptr<Attr>> lexicographicOrder(const SymbolTable & symbols) const
    {
        std::vector<std::unique_ptr<Attr>> res;
        res.reserve(size_);

        for (size_t n = 0; n < size_; n++)
        {
            auto r = this->attrDiskCache_[range_start_ + n];
            auto e = std::unique_ptr<Attr>(new Attr(r.name, r.value, r.pos));
            res.emplace_back(std::move(e));
        }
        std::sort(res.begin(), res.end(), [&](const std::unique_ptr<Attr>& a, const std::unique_ptr<Attr>& b) {
            std::string_view sa = symbols[a->name], sb = symbols[b->name];
            return sa < sb;
        });
        return res;
    }

    friend class EvalState;
};

/**
 * A wrapper around Bindings that ensures that its always in sorted
 * order at the end. The only way to consume a BindingsBuilder is to
 * call finish(), which sorts the bindings.
 */
class BindingsBuilder
{
    Bindings * bindings;

public:
    // needed by std::back_inserter
    using value_type = Attr;

    EvalState & state;

    BindingsBuilder(EvalState & state, Bindings * bindings)
        : bindings(bindings), state(state)
    { }

    void insert(Symbol name, Value * value, PosIdx pos = noPos)
    {
        insert(Attr(name, value, pos));
    }

    void insert(const Attr & attr)
    {
        push_back(attr);
    }

    void push_back(const Attr & attr)
    {
        bindings->push_back(attr);
    }

    Value & alloc(Symbol name, PosIdx pos = noPos);

    Value & alloc(std::string_view name, PosIdx pos = noPos);

    Bindings * finish()
    {
        bindings->sort();
        return bindings;
    }

    Bindings * alreadySorted()
    {
        return bindings;
    }
};

}
