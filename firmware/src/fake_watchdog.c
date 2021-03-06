#include "fake_watchdog.h"

FakeWatchdog dogs[] = {
    {
        .tmrModule = RIGHT_TMR_MODULE,
        .intSource = RIGHT_TMR_SOURCE,
        .intVector = RIGHT_TMR_VECTOR,
        .enable = false,
        .initialized = false,
        .callback = NULL,
    },
    {
        .tmrModule = LEFT_TMR_MODULE,
        .intSource = LEFT_TMR_SOURCE,
        .intVector = LEFT_TMR_VECTOR,
        .enable = false,
        .initialized = false,
        .callback = NULL,
    }
};

inline void manageCallbackFakeWD(const ChannelIndex idx) {
    FakeWatchdog *p = FakeWD_Get(idx);
    if(p && p->callback)
        p->callback(idx, p->context);
    PLIB_INT_SourceFlagClear(INT_ID_0, p->intSource);
}

// Callbacks
void __ISR(RIGHT_TMR_ISR, IPL7AUTO) fakewd_right() {
    manageCallbackFakeWD(Channel_Right);
}

void __ISR(LEFT_TMR_ISR, IPL7AUTO) fakewd_left() {
    manageCallbackFakeWD(Channel_Left);
}


FakeWatchdog *FakeWD_Get(const ChannelIndex index){
    if(!IS_VALID_CHANNEL_IDX(index))
        return NULL;
    return FakeWD_Initialize(index);
}

FakeWatchdog *FakeWD_Initialize(const ChannelIndex dog){
    FakeWatchdog *p = &dogs[dog];

    if(p->initialized)
        return p;

    TMR_MODULE_ID tmr = p->tmrModule;

    PLIB_TMR_Stop(tmr);
    PLIB_TMR_ClockSourceSelect(tmr, TMR_CLOCK_SOURCE_PERIPHERAL_CLOCK);
    if(PLIB_TMR_ExistsMode32Bit(tmr)) 
        PLIB_TMR_Mode16BitEnable(tmr);
    PLIB_TMR_PrescaleSelect(tmr, TMR_PRESCALE_VALUE_8);

    // Period set to 550 uS
    PLIB_TMR_Period16BitSet(tmr, 2750);

    // Interrupt
    PLIB_INT_SourceFlagClear(INT_ID_0, p->intSource);
    PLIB_INT_SourceEnable(INT_ID_0, p->intSource);
    PLIB_INT_VectorPrioritySet(INT_ID_0, p->intVector, INT_PRIORITY_LEVEL7);

    p->initialized = true;

    return p;
}

void FakeWD_Enable(FakeWatchdog *dog, const bool enable){
    if(!dog || !dog->initialized)
        return;
    dog->enable = enable;
    if(enable)
        PLIB_TMR_Start(dog->tmrModule);
    else
        PLIB_TMR_Stop(dog->tmrModule);
}

void FakeWD_Kick(FakeWatchdog *dog){
    if(!dog)
        return;
    PLIB_TMR_Counter16BitSet(dog->tmrModule, 0);
}

void FakeWD_SetCallback(FakeWatchdog *dog, FAKE_WD_EXPIRED callback, uintptr_t *context){
    if(!dog)
        return;
    dog->callback= callback;
    dog->context = context;
}

