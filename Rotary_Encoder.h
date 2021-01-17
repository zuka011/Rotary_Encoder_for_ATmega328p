#ifndef _Rotary_Encoder_h
#define _Rotary_Encoder_h

#include "Arduino.h"

/* The following header file defines a struct used for working with a rotary encoder, the KY-040 in particular */

enum Direction {Clockwise, CounterClockwise};

struct Encoder {
public:

    // DISCLAIMER: This library overrides the compare match A interrupt of Timer0.

    /** 
     * Constructor:
     * Creates an Encoder object that is associated with a rotary encoder by specifying the respective A and B pins.
     *
     * !!YOU NEED TO CALL init() BEFORE USING OTHER ENCODER METHODS!!
     */
    Encoder(uint8_t pinA, uint8_t pinB);
    ~Encoder();

    /**
     * init();
     * ------------------------------------------------------------------------------------------------
     * Initializes the control pins as inputs and enables the timer interrupt.
     */
    void init();


    /**
     * enable();
     * ------------------------------------------------------------------------------------------------
     * Enables the rotary encoders functioning. This is called with init() by default, but can be used
     * separately, in case disable() was called.
     */
    void enable();

    /**
     * disable();
     * ------------------------------------------------------------------------------------------------
     * Disables the rotary encoders functioning. Can be re-enabled by calling enable().
     */
    void disable();

    /**
     * attachFunction();
     * ------------------------------------------------------------------------------------------------
     * On encoder rotations the user attached functions will be called. [direction] specifies which 
     * direction of rotation should trigger a call to the specified function. You can have different 
     * function calls set for different directions.
     */
    void attachFunction(void (*func)(void), Direction direction);

    /**
     * getTickCount();
     * ------------------------------------------------------------------------------------------------
     * Returns the number of ticks(rotation steps) occured after the Encoder object was initialized. Every
     * clockwise rotation increments this count and every counter-clockwise rotation decrements it, so the 
     * result may be a negative number.
     */
    long getTickCount();

    /**
     * getLastDirection();
     * ------------------------------------------------------------------------------------------------
     * Returns the direction of the last encoder rotation as an enumerated type.
     */
    Direction getLastDirection();

    /**
     * checkEncoder();
     * ------------------------------------------------------------------------------------------------
     * This is used by the timer ISR functions. You will not need to call this method directly.
     */
    void checkEncoder();

private:

    uint8_t pinA, pinB;
    uint8_t pinABit, pinBBit;
    uint8_t *PINRegisterA;
    uint8_t *PINRegisterB;

    long tickCount;
    Direction turnDirection;

    void (*clockWiseFunc)(void);
    void (*cClockWiseFunc)(void);

    uint8_t *getPin(uint8_t &pin);

    void initTimer();
};

#endif 