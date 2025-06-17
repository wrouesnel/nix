#include "value.hh"

namespace nix
{
auto Value::listItems()
{
    struct ListIterable
    {
        typedef Value * const * iterator;
        iterator _begin, _end;
        iterator begin() const { return _begin; }
        iterator end() const { return _end; }
    };
    assert(isList());
    auto begin = listElems();
    return ListIterable { begin, begin + listSize() };
}

auto Value::listItems() const
{
    struct ConstListIterable
    {
        typedef const Value * const * iterator;
        iterator _begin, _end;
        iterator begin() const { return _begin; }
        iterator end() const { return _end; }
    };
    assert(isList());
    auto begin = listElems();
    return ConstListIterable { begin, begin + listSize() };
}

SourcePath Value::path() const
{
    assert(internalType == tPath);
    return SourcePath{CanonPath(_path)};
}

std::string_view Value::str() const
{
    assert(internalType == tString);
    return std::string_view(string.s);
}

bool Value::isList() const
{
    return internalType == tList1 || internalType == tList2 || internalType == tListN;
}

}