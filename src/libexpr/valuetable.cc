#include "value.hh"
#include "../libstore/path-with-outputs.hh"

namespace nix
{
struct SourcePath;

void ValueTable::ValueTable()
{
    nrValues = 0;
    values.allocate_page_cache();
}

void ValueTable::~ValueTable()
{
    values.deallocate_page_cache();
}

ValueIdx ValueTable::getNextIndex()
{
    return nrValues++;
}

ValueIdx ValueTable::writeValue(Value value)
{
    auto nextIdx = getNextIndex();
    auto offset = values.size();
    values.push_back(value);
    valuesMap[nextIdx] = offset;
    return nextIdx;
}

Value * * ValueTable::listElems()
{
    return internalType == tList1 || internalType == tList2 ? smallList : bigList.elems;
}

const Value * const * ValueTable::listElems() const
{
    return internalType == tList1 || internalType == tList2 ? smallList : bigList.elems;
}

size_t ValueTable::listSize() const
{
    return internalType == tList1 ? 1 : internalType == tList2 ? 2 : bigList.size;
}

/**
 * After overwriting an app node, be sure to clear pointers in the
 * Value to ensure that the target isn't kept alive unnecessarily.
 */
ValueIdx ValueTable::clearValue(ValueIdx idx)
{
    auto currentPos = valuesMap[idx];
    auto value = values[currentPos];
    value.app.left = 0;
    value.app.right = 0;
    auto currentPos = writeValue(value);
    valuesMap[idx] = newIdx;
    return idx;
}

ValueIdx ValueTable::mkInt(NixInt n)
{
    auto v = Value();
    v.internalType = tInt;
    v.integer = n;
    return writeValue(v);
}

ValueIdx ValueTable::mkBool(bool b)
{
    auto v = Value();
    v.internalType = tBool;
    v.boolean = b;
    return writeValue(v);
}

ValueIdx ValueTable::mkString(const char * s, const char ** context = 0)
{
    auto v = Value();
    v.internalType = tString;
    v.string.s = s;
    v.string.context = context;
    return writeValue(v);
}

ValueIdx ValueTable::mkString(std::string_view s)
{
    return mkString(makeImmutableString(s));
}

ValueIdx ValueTable::mkString(std::string_view s, const NixStringContext & context)
{
    auto r = mkString(s);
    copyContextToValue(*this, context);
    return r;
}

ValueIdx mkString(const Symbol & s)
{
    return mkString(((const std::string &) s).c_str());
}

ValueIdx ValueTable::mkPath(const SourcePath & path)
{
    return mkPath(makeImmutableString(path.path.abs()));
}

ValueIdx ValueTable::mkPath(const char * path)
{
    auto v = Value();
    v.internalType = tPath;
    v._path = path;
    return writeValue(v);
}

ValueIdx ValueTable::mkNull()
{
    auto v = Value();
    v.internalType = tNull;
    return writeValue(v);
}

ValueIdx ValueTable::mkAttrs(Bindings * a)
{
    auto v = Value();
    v.internalType = tAttrs;
    v.attrs.start = a->start_;
    v.attrs.end = a->end_;
    return writeValue(v);
}

// ValueIdx ValueTable::mkAttrs(BindingsBuilder & bindings)
// {
//     return mkAttrs(bindings.finish());
// }

ValueIdx ValueTable::mkList(size_t size)
{
    // Because of the disk cache, all lists can be stored as the same list type
    // and we simply sequentially allocate a vector for the size.
    ValueIdx listStart = mkNull();
    ValueIdx listEnd;
    for (auto i=1;i<size;++i)
    {
        listEnd = mkNull();
    }

    // Got our linear range of values, now allocate the list.

    auto v = Value();
    v.internalType = tListN;
    v.list.start = listStart;
    v.list.end = listEnd;
    return writeValue(v);
}

ValueIdx ValueTable::mkThunk(Env * e, Expr * ex)
{
    auto v = Value();
    v.internalType = tThunk;
    v.thunk.env = e;
    v.thunk.expr = ex;
    return writeValue(v);
}

ValueIdx ValueTable::mkApp(Value * l, Value * r)
{
    auto v = Value();
    v.internalType = tApp;
    v.app.left = l;
    v.app.right = r;
    return writeValue(v);
}

ValueIdx ValueTable::mkLambda(Env * e, ExprLambda * f)
{
    auto v = Value();
    v.internalType = tLambda;
    v.lambda.env = e;
    v.lambda.fun = f;
    return writeValue(v);
}

ValueIdx ValueTable::mkBlackhole()
{
    auto v = Value();
    v.internalType = tBlackhole;
    // Value will be overridden anyways
    return writeValue(v);
}

ValueIdx ValueTable::mkPrimOp(PrimOp * p)
{
    auto v = Value();
    v.internalType = tPrimOp;
    v.primOp = p;
    return writeValue(v);
}


ValueIdx ValueTable::mkPrimOpApp(Value * l, Value * r)
{
    auto v = Value();
    v.internalType = tPrimOpApp;
    v.app.left = l;
    v.app.right = r;
    return writeValue(v);
}

ValueIdx ValueTable::mkExternal(ExternalValueBase * e)
{
    auto v = Value();
    v.internalType = tExternal;
    v.external = e;
    return writeValue(v);
}

ValueIdx ValueTable::mkFloat(NixFloat n)
{
    auto v = Value();
    v.internalType = tFloat;
    v.fpoint = n;
    return writeValue(v);
}

}
