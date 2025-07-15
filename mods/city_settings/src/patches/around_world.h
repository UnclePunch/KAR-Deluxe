#ifndef CITYSETTINGS_AROUNDWORLD_H
#define CITYSETTINGS_AROUNDWORLD_H

#define STADIUMS_TO_PLAY 3

void AroundWorld_Init();
void AroundWorld_DecideStadiums();
void AroundWorld_ApplyPatches();
void AroundWorld_EqualizeItemChances();
void AroundWorld_CalculateRanks(s8 *ranks, void *score, char data_type, int is_higher_better);

#endif