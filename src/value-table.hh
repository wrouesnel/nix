#pragma once

#include <limits>
#include <stxxl/map>

#include "value-table-types.hh"
#include "libexpr/value.hh"

#define DATA_NODE_BLOCK_SIZE (4096)
#define DATA_LEAF_BLOCK_SIZE (4096)

namespace nix
{
struct ValueIdxCompareLess
{
    bool operator () (const ValueIdx & a, const ValueIdx & b) const
    { return a<b; }
    static size_t max_value()
    { return std::numeric_limits<ValueIdx>::max(); }
};

/**
 * ValueTable centralizes the management of values, so they can be created and pageD out to disk backed
 * cache by STXXL. EvalState carries a ValueTable, which in turn keeps our memory usage under control by
 * allowing things to be paged to disk.
 */
class ValueTable
{
private:
    stxxl::map<ValueIdx,ValueOffset,ValueIdxCompareLess,DATA_NODE_BLOCK_SIZE,DATA_LEAF_BLOCK_SIZE> valuesMap;
    stxxl::vector<Value> values;

    ValueIdx nrValues;

    // Write a new value to the map
    ValueIdx writeValue(Value value);

    // Get the next value index (for future thread safing)
    ValueIdx getNextIndex();

public:
    ValueTable();
    ~ValueTable();

    ValueIdx mkInt(NixInt n);
    ValueIdx mkBool(bool b);

    ValueIdx mkString(const char * s, const char ** context = 0);
    ValueIdx mkString(std::string_view s);
    ValueIdx mkString(std::string_view s, const NixStringContext & context);
    ValueIdx mkString(const Symbol & s);

    ValueIdx mkPath(const SourcePath & path);

    ValueIdx mkNull();

    ValueIdx mkAttrs(Bindings * a);
    // ValueIdx mkAttrs(BindingsBuilder & bindings);

    ValueIdx mkList(size_t size);

    ValueIdx mkThunk(Env * e, Expr * ex);
    ValueIdx mkApp(ValueIdx l, ValueIdx r);
    ValueIdx mkLambda(Env * e, ExprLambda * f);
    ValueIdx mkBlackhole();
    ValueIdx mkPrimOp(PrimOp * p);
    ValueIdx mkPrimOpApp(ValueIdx l, ValueIdx r);
    ValueIdx mkExternal(ExternalValueBase * e);
    ValueIdx mkFloat(NixFloat n);

    // Technically not needed because we no longer use GC.
    ValueIdx clearValue(ValueIdx idx);

    size_t size() const
    {
        return values.size();
    }

    size_t totalSize() const;
};
}