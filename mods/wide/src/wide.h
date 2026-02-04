#ifndef MOD_H_WIDE
#define MOD_H_WIDE

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

typedef enum WideKind
{
    WIDEKIND_43,
    WIDEKIND_169,
    WIDEKIND_1610,
    WIDEKIND_329,
} WideKind;

void Wide_Init();
void Wide_OnOptionChange(int val);
void Wide_CreateTestGObj();
void CObj_AdjustWideOrtho(COBJ *c);

#endif