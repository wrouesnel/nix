#pragma once

/**
 * Keep these separate from value.hh to avoid circular dependencies
 */
typedef int64_t NixInt;
typedef double NixFloat;

/**
 * ValueIdx is the unique identifier for a value in the lookup table. ValueIdx is mapped through the ValueTable
 * to the current actual value of the Value. This implements a copy on write scheme in the master value cache
 * which hopefully keeps in-use values local (and allows to update them since we can just extend the buffer).
 */
typedef size_t ValueIdx;
typedef size_t ValueOffset;