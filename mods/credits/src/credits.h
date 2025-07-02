#ifndef CREDITS_H
#define CREDITS_H

#define TEXT_ARRAY_SIZE 5
#define CREDIT_NUM 10
#define SCROLL_SPEED (0.75)

typedef struct ScrollData
{
    int is_end;
    int timer;
    int credit_idx;
    Text *text[TEXT_ARRAY_SIZE];
} ScrollData;

MajorKind Credits_Init();

void CreditsMajor_Enter();
void CreditsMajor_Exit();

void CreditsMinor_Enter();
void CreditsMinor_Exit(void *data);
void CreditsMinor_Think(void *data);

GOBJ *Credits_ScrollCreate();
void Credits_ScrollThink(GOBJ *g);
void Credits_ScrollDestroy(ScrollData *sp);
#endif