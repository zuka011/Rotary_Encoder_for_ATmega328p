#include "Arduino.h"
#include "Rotary_Encoder.h"

enum State {Start = 0, CWStart, CWMid, CWEnd, CCWStart, CCWMid, CCWEnd};

static const uint8_t MAX_ENCODERS = 8;
static Encoder *callback_encoders[MAX_ENCODERS];
static uint8_t last_encoder;

Encoder::Encoder(uint8_t pinA, uint8_t pinB) {

    this->pinA = pinA;
    this->pinB = pinB;

    pinABit = pinA;
    pinBBit = pinB;

    PINRegisterA = getPin(pinABit);
    PINRegisterB = getPin(pinBBit);
}

Encoder::~Encoder() {
    disable();
}

void Encoder::init() {

    pinMode(pinA, INPUT);
    pinMode(pinB, INPUT);

    tickCount = 0;
    clockWiseFunc = NULL;
    cClockWiseFunc = NULL;

    initTimer();
    enable();
}

void Encoder::attachFunction(void (*func)(void), Direction direction) {

    if(direction == Clockwise) clockWiseFunc = func;
    else cClockWiseFunc = func;
}

long Encoder::getTickCount() {
    return tickCount;
}

Direction Encoder::getLastDirection() {
    return turnDirection;
}

void Encoder::checkEncoder() {

    static volatile State state = Start;
    static volatile bool encoderAState = false, encoderBState = false;

    encoderAState = (*PINRegisterA) & _BV(pinABit); 
    encoderBState = (*PINRegisterB) & _BV(pinBBit);

    switch (state) {

    case Start:
        if (!encoderAState && !encoderBState) return;

        if (encoderAState && !encoderBState) state = CWStart;
        if (!encoderAState && encoderBState) state = CCWStart;

        break;
    case CWStart:

        if (encoderAState && encoderBState) state = state + 1;
        if (!encoderAState && !encoderBState) state = Start;
        break;

    case CWMid:

        if (!encoderAState && encoderBState) state = state + 1;
        if (encoderAState && !encoderBState) state = state - 1;
        break;
    case CWEnd:

        if (!encoderAState && !encoderBState) {

            tickCount++;
            turnDirection = Clockwise;

            if(clockWiseFunc != NULL) clockWiseFunc();
            state = Start;
        }
        if (encoderAState && encoderBState) state = state - 1;
        break;
    case CCWStart:

        if (encoderAState && encoderBState) state = state + 1;
        if (!encoderAState && !encoderBState) state = Start;

        break;
    case CCWMid:

        if (encoderAState && !encoderBState) state = state + 1;
        if (!encoderAState && encoderBState) state = state - 1;

        break;
    case CCWEnd:

        if (!encoderAState && !encoderBState) {

            tickCount--;
            turnDirection = CounterClockwise;

            if(cClockWiseFunc != NULL) cClockWiseFunc();
            state = Start;
        }
        if (encoderAState && encoderBState) state = state - 1;
        break;
    }
}

void Encoder::initTimer() {
    
    if(TCCR0B & B00000111 == 0 ) TCCR0B |= _BV(CS01);

    OCR0A = B11111111 / 2;    
    TIMSK0 |= _BV(OCIE0A);
}

void Encoder::enable() {

    if(last_encoder == MAX_ENCODERS) return;
    for(int i = 0; i < last_encoder; i++) if(callback_encoders[i] == this) return;

    callback_encoders[last_encoder++] = this;
}

void Encoder::disable() {
    
    int removedEncoderIndex = -1;

    for(int i = 0; i < last_encoder; i++) {

        if (callback_encoders[i] == this) {

            removedEncoderIndex = i;
            break;
        }
    }

    if(removedEncoderIndex != -1) {

        for(int i = removedEncoderIndex + 1; i < last_encoder; i++) callback_encoders[i - 1] = callback_encoders[i];
        last_encoder--;
    }
}

uint8_t *Encoder::getPin(uint8_t &pin) {

    switch(pin) {
    
        case 0: pin = PD0; return &PIND;
        case 1: pin = PD1; return &PIND;
        case 2: pin = PD2; return &PIND;
        case 3: pin = PD3; return &PIND;
        case 4: pin = PD4; return &PIND;
        case 5: pin = PD5; return &PIND;
        case 6: pin = PD6; return &PIND;
        case 7: pin = PD7; return &PIND;

        case 8: pin = PB0; return &PINB;
        case 9: pin = PB1; return &PINB;
        case 10: pin = PB2; return &PINB;
        case 11: pin = PB3; return &PINB;
        case 12: pin = PB4; return &PINB;
        case 13: pin = PB5; return &PINB;

        case 14: pin = PC0; return &PINC;
        case 15: pin = PC1; return &PINC;
        case 16: pin = PC2; return &PINC;
        case 17: pin = PC3; return &PINC;
        case 18: pin = PC4; return &PINC;
        case 19: pin = PC5; return &PINC;
    }

    return NULL;
}

ISR(TIMER0_COMPA_vect) {

    for(int i = 0; i < last_encoder; i++) callback_encoders[i]->checkEncoder();
}