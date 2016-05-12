#ifndef BGAME_ATTACK2_H
#define BGAME_ATTACK2_H

///////////////////////////////////////////////////////////////////////////////

/* This number cannot be changed without also changing the following:
 *
 * - animation defs for each level: animations/human/base/body.aninc
 * - script actions for each level/weapon: animations/scripts/human_base.script
 * - appropriate population of array entries for each supported weapon
 */
#define A2_ARMING_LEVELS 10

///////////////////////////////////////////////////////////////////////////////

typedef enum {
    A2_ALT_IDLE,
    A2_ALT_CLICKED,
} a2_alt_state_t;

typedef struct a2_alt_s {
    a2_alt_state_t state;
    int            stateAlarm; // block transition until alarm is satisfied
} a2_alt_t;

///////////////////////////////////////////////////////////////////////////////

typedef enum {
    A2_THROWINGKNIFE_IDLE,
    A2_THROWINGKNIFE_ARMING,
    A2_THROWINGKNIFE_THROWING,
    A2_THROWINGKNIFE_RELEASE,
} a2_throwingKnife_state_t;

typedef struct a2_throwingKnife_level_s {
    weapAnimNumber_t       first;          // 1st-person animation
    scriptAnimEventTypes_t third;          // 3rd-person animation
    int                    throwingPeriod; // time to block when throwing (ms)
} a2_throwingKnife_level_t;

typedef struct a2_throwingKnife_s {
    a2_throwingKnife_state_t  state;
    int                       stateAlarm;    // block transition until alarm is satisfied
    int                       depressedTime; // when attack2 button was depressed
    int                       armingIndex;
    a2_throwingKnife_level_t* armingData;
} a2_throwingKnife_t;

///////////////////////////////////////////////////////////////////////////////

typedef enum {
    A2_MOLOTOV_IDLE,
    A2_MOLOTOV_ARMING,
    A2_MOLOTOV_THROWING,
    A2_MOLOTOV_RELEASE,
} a2_molotov_state_t;

typedef struct a2_molotov_level_s {
    weapAnimNumber_t       first;          // 1st-person animation
    scriptAnimEventTypes_t third;          // 3rd-person animation
    int                    throwingPeriod; // time to block when throwing (ms)
} a2_molotov_level_t;

typedef struct a2_molotov_s {
    a2_molotov_state_t  state;
    int                       stateAlarm;    // block transition until alarm is satisfied
    int                       depressedTime; // when attack2 button was depressed
    int                       armingIndex;
    a2_molotov_level_t* armingData;
} a2_molotov_t;

///////////////////////////////////////////////////////////////////////////////

extern const char* a2_alt_state_names[];
extern const char* a2_molotov_state_names[];
extern const char* a2_throwingKnife_state_names[];

extern a2_molotov_level_t       a2_molotov_level_table[A2_ARMING_LEVELS];
extern a2_throwingKnife_level_t a2_throwingKnife_level_table[A2_ARMING_LEVELS];

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_ATTACK2_H
