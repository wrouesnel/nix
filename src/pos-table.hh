#pragma once
#include <algorithm>
#include <iosfwd>
#include <variant>

#include "libfetchers/input-accessor.hh"
#include "libutil/chunked-vector.hh"
#include "libutil/error.hh"
#include "libutil/ref.hh"

namespace nix
{

/**
 * Position objects.
 */
struct Pos
{
    uint32_t line;
    uint32_t column;

    struct none_tag { };
    struct Stdin { ref<std::string> source; };
    struct String { ref<std::string> source; };

    typedef std::variant<none_tag, Stdin, String, SourcePath> Origin;

    Origin origin;

    explicit operator bool() const { return line > 0; }

    operator std::shared_ptr<AbstractPos>() const;
};

class PosIdx {
    friend class PosTable;

private:
    uint32_t id;

    explicit PosIdx(uint32_t id): id(id) {}

public:
    PosIdx() : id(0) {}

    explicit operator bool() const { return id > 0; }

    bool operator <(const PosIdx other) const { return id < other.id; }

    bool operator ==(const PosIdx other) const { return id == other.id; }

    bool operator !=(const PosIdx other) const { return id != other.id; }
};

class PosTable
{
public:
    class Origin {
        friend PosTable;
    private:
        // must always be invalid by default, add() replaces this with the actual value.
        // subsequent add() calls use this index as a token to quickly check whether the
        // current origins.back() can be reused or not.
        mutable uint32_t idx = std::numeric_limits<uint32_t>::max();

        // Used for searching in PosTable::[].
        explicit Origin(uint32_t idx): idx(idx), origin{Pos::none_tag()} {}

    public:
        const Pos::Origin origin;

        Origin(Pos::Origin origin): origin(origin) {}
    };

    struct Offset {
        uint32_t line, column;
    };

private:
    std::vector<Origin> origins;
    ChunkedVector<Offset, 8192> offsets;

public:
    PosTable(): offsets(1024)
    {
        origins.reserve(1024);
    }

    PosIdx add(const Origin & origin, uint32_t line, uint32_t column)
    {
        const auto idx = offsets.add({line, column}).second;
        if (origins.empty() || origins.back().idx != origin.idx) {
            origin.idx = idx;
            origins.push_back(origin);
        }
        return PosIdx(idx + 1);
    }

    Pos operator[](PosIdx p) const
    {
        if (p.id == 0 || p.id > offsets.size())
            return {};
        const auto idx = p.id - 1;
        /* we want the last key <= idx, so we'll take prev(first key > idx).
           this is guaranteed to never rewind origin.begin because the first
           key is always 0. */
        const auto pastOrigin = std::upper_bound(
            origins.begin(), origins.end(), Origin(idx),
            [] (const auto & a, const auto & b) { return a.idx < b.idx; });
        const auto origin = *std::prev(pastOrigin);
        const auto offset = offsets[idx];
        return {offset.line, offset.column, origin.origin};
    }
};

inline PosIdx noPos = {};

std::ostream & operator << (std::ostream & str, const Pos & pos);
}
