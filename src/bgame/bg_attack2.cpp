#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

const char* a2_alt_state_names[] = { 
    "IDLE",
    "CLICKED",
};

const char* a2_molotov_state_names[] = { 
    "IDLE",
    "ARMING",
    "THROWING",
    "RELEASE",
};

const char* a2_throwingKnife_state_names[] = { 
    "IDLE",
    "ARMING",
    "THROWING",
    "RELEASE",
};

///////////////////////////////////////////////////////////////////////////////

a2_molotov_level_t a2_molotov_level_table[A2_ARMING_LEVELS] = {
//    -------------------------------------
//    FIRST       THIRD               THROW
//    enum        enum                 (ms)
//    -------------------------------------
    { WEAP_EXT_0, ANIM_ET_OVERHAND_0,   600 },
    { WEAP_EXT_1, ANIM_ET_OVERHAND_1,   541 },
    { WEAP_EXT_2, ANIM_ET_OVERHAND_2,   482 },
    { WEAP_EXT_3, ANIM_ET_OVERHAND_3,   422 },
    { WEAP_EXT_4, ANIM_ET_OVERHAND_4,   363 },
    { WEAP_EXT_5, ANIM_ET_OVERHAND_5,   304 },
    { WEAP_EXT_6, ANIM_ET_OVERHAND_6,   245 },
    { WEAP_EXT_7, ANIM_ET_OVERHAND_7,   185 },
    { WEAP_EXT_8, ANIM_ET_OVERHAND_8,   126 },
    { WEAP_EXT_9, ANIM_ET_OVERHAND_9,    67 },
};

a2_throwingKnife_level_t a2_throwingKnife_level_table[A2_ARMING_LEVELS] = {
//    -------------------------------------
//    FIRST       THIRD               THROW
//    enum        enum                 (ms)
//    -------------------------------------
    { WEAP_EXT_0, ANIM_ET_OVERHAND_0,   600 },
    { WEAP_EXT_1, ANIM_ET_OVERHAND_1,   541 },
    { WEAP_EXT_2, ANIM_ET_OVERHAND_2,   482 },
    { WEAP_EXT_3, ANIM_ET_OVERHAND_3,   422 },
    { WEAP_EXT_4, ANIM_ET_OVERHAND_4,   363 },
    { WEAP_EXT_5, ANIM_ET_OVERHAND_5,   304 },
    { WEAP_EXT_6, ANIM_ET_OVERHAND_6,   245 },
    { WEAP_EXT_7, ANIM_ET_OVERHAND_7,   185 },
    { WEAP_EXT_8, ANIM_ET_OVERHAND_8,   126 },
    { WEAP_EXT_9, ANIM_ET_OVERHAND_9,    67 },
};
