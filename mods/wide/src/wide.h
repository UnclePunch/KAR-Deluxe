#ifndef MOD_H_WIDE
#define MOD_H_WIDE

#define WIDE_VERSION_MAJOR 1
#define WIDE_VERSION_MINOR 0

// this is the width of the original perspective screen camera
#define ORIG_WIDTH (60.68)
#define ORIG_HEIGHT (48.36)
#define ORIG_ASPECT (1.255375)

typedef enum WideKind
{
    WIDEKIND_43,
    WIDEKIND_169,
    WIDEKIND_1610,
    WIDEKIND_329,
} WideKind;

void Wide_Init();
float Wide_GetAspectMult();
void Wide_OnOptionChange(int val);
void Wide_CreateTestGObj();
void CObj_AdjustWideOrtho(COBJ *c);

#endif