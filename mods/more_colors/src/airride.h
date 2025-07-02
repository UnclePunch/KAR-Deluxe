#ifndef MORECOLORS_H_AIRRIDE
#define MORECOLORS_H_AIRRIDE

void AirRideSelect_Init();
void AirRideSelect_UpdatePlayer(int ply, int p_kind, int color_idx);
void AirRideSelect_Cursor0Update(int ply, int color_idx);
void AirRideSelect_Cursor0Think(GOBJ *g);
void AirRideSelect_Cursor1Update(int ply, int color_idx);
void AirRideSelect_Cursor5Update(int ply, int color_idx);
void AirRideSelect_Cursor5Think(GOBJ *g);
void AirRideSelect_Cursor6Think(GOBJ *g);

#endif