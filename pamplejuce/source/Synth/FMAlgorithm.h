#pragma once

static constexpr int FM_OP_COUNT = 4;
static constexpr int FM_ALGORITHM_COUNT = 8;

// Describes the routing for a 4-operator FM algorithm.
// Operators are indexed 0-3. Processing runs 3→2→1→0 so higher-indexed
// operators are always resolved before the lower-indexed ones they feed into.
// modulatedBy[i] is a bitmask: bit j set means operator j modulates operator i.
struct FMAlgorithm
{
    const char* name;
    bool isCarrier[FM_OP_COUNT];
    int modulatedBy[FM_OP_COUNT]; // bitmask of source operators for each op
};

// clang-format off
static constexpr FMAlgorithm FM_ALGORITHMS[FM_ALGORITHM_COUNT] = {
    // 0: Series chain  3→2→1→0  — classic one-carrier, three-modulator stack
    {
        "Series Chain",
        { true,  false, false, false },
        { 1<<1,  1<<2,  1<<3,  0 }
    },
    // 1: Two modulators into one carrier  3→1→0, 2→0
    {
        "Twin Mod",
        { true,  false, false, false },
        { (1<<1)|(1<<2), 1<<3, 0, 0 }
    },
    // 2: Two independent stacks  3→2, 1→0  — two carriers
    {
        "Two Stacks",
        { true,  false, true,  false },
        { 1<<1,  0,     1<<3,  0 }
    },
    // 3: One modulator feeds three carriers  3→(0,1,2)
    {
        "Mod Fork",
        { true,  true,  true,  false },
        { 1<<3,  1<<3,  1<<3,  0 }
    },
    // 4: Additive — all four operators are carriers, no modulation
    {
        "Additive",
        { true,  true,  true,  true  },
        { 0,     0,     0,     0 }
    },
    // 5: Stack of three + solo carrier  3→2→1, 0 alone
    {
        "Stack+Solo",
        { true,  true,  false, false },
        { 0,     1<<2,  1<<3,  0 }
    },
    // 6: Chain with fork at the end  3→2→(0,1)  — two carriers share one modulator chain
    {
        "Chain Fork",
        { true,  true,  false, false },
        { 1<<2,  1<<2,  1<<3,  0 }
    },
    // 7: Diamond  3→2→0, 3→1→0  — op3 drives two parallel paths into op0
    {
        "Diamond",
        { true,  false, false, false },
        { (1<<1)|(1<<2), 1<<3, 1<<3, 0 }
    }
};
// clang-format on
