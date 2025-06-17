#pragma once
#include "pos-table.hh"
#include "symbol-table.hh"
#include "value-table-types.hh"

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
    ValueIdx value;
    Attr(Symbol name, ValueIdx value, PosIdx pos = noPos)
        : name(name), pos(pos), value(value) { };
    Attr() { };
    bool operator < (const Attr & a) const
    {
        return name < a.name;
    }
};

static_assert(sizeof(Attr) == 2 * sizeof(uint32_t) + sizeof(ValueIdx),
    "performance of the evaluator is highly sensitive to the size of Attr. "
    "avoid introducing any padding into Attr if at all possible, and do not "
    "introduce new fields that need not be present for almost every instance.");

}
