#pragma once

#include "attr.hh"

namespace nix
{
typedef size_t AttrIdx;

/**
 * AttributesTable provides a master lookup table for attributes. This notably just means
 * storing them all in a big index vector on disk, so bindings can store vector ranges.
 * rather then lists of attributes.
 */
class AttributesTable
{
private:
    typedef stxxl::vector<Attr> AttributeCache;
    AttributeCache attrs;
public:
    AttributesTable()
    {
        attrs.allocate_page_cache();
    }
    ~AttributesTable()
    {
        attrs.deallocate_page_cache();
    }

    // ValueIdx create(std::unique_ptr<Attr> attr)
    // {
    //     AttrIdx idx = attrs.size();
    //     attrs[idx] = *attr.get();
    //     return idx;
    // }
    //
    // std::unique_ptr<Attr> operator[](const AttrIdx idx)
    // {
    //     auto stored_value = attrs[idx];
    //     Attr* rvalue = new Attr(); // This is not leaked - we hand it to the unique ptr below.
    //     memcpy(rvalue, &stored_value, sizeof(Attr));
    //     return std::unique_ptr<Attr>(rvalue);
    // }

    size_t size() const
    {
        return attrs.size();
    }

    size_t totalSize() const;
};
}