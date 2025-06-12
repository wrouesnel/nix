#pragma once
///@file

#include <stxxl/vector>

namespace nix
{

/**
 * Map one attribute name to its value.
 */
struct Attr
{
    /* the placement of `name` and `pos` in this struct is important.
       both of them are uint32 wrappers, they are next to each other
       to make sure that Attr has no padding on 64 bit machines. that
       way we keep Attr size at two words with no wasted space. */
    Symbol name;
    PosIdx pos;
    Value * value;
    Attr(Symbol name, Value * value, PosIdx pos = noPos)
        : name(name), pos(pos), value(value) { };
    Attr() { };
    bool operator < (const Attr & a) const
    {
        return name < a.name;
    }
};

typedef stxxl::vector<Attr> bigAttrCache;

}
