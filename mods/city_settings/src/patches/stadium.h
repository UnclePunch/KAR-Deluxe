#ifndef CITYSETTINGS_STADIUM_H
#define CITYSETTINGS_STADIUM_H

typedef enum StadiumSelection
{
    STADSELECT_SHUFFLE,
    STADSELECT_ALL,
    STADSELECT_DRAGRACE,
    STADSELECT_AIRGLIDER,
    STADSELECT_TARGETFLIGHT,
    STADSELECT_HIGHJUMP,
    STADSELECT_MELEE,
    STADSELECT_DESTRUCTION,
    STADSELECT_SINGLERACE,
    STADSELECT_VSKINGDEDEDE,
} StadiumSelection;

struct StadiumChance
{
    StadiumKind kind;
    int chance;
};

void Stadium_ApplyPatches();

#endif