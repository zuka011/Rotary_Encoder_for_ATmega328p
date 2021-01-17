#ifndef _Buzzer_h
#define _Buzzer_h

#include "Arduino.h"

#define INDEFINITE -1
#define REST_NOTE 0

enum Letter {C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B}; // Note names in 12TET system.

struct Note { // A struct representing a musical note, with it's name and octave (12TET system).

    Letter name;
    uint8_t octave;

    Note(Letter name = C, uint8_t octave = 0) : name(name), octave(octave) {};

    String toString();
};

bool operator==(const Note & note1, const Note & note2);

const Note REST(0, 0);

class Buzzer {
public:

    // DISCLAIMER: This library overrides the input capture interrupt of Timer1. Also heavily modifies the timing of other interrupts and the prescaler.
    // WARNING: This library only works if you connect the buzzer to either pin 9 or pin 10.

    /** 
     * Constructor:
     * Creates a Buzzer object that can be associated with a piezo buzzer or any other kind of speaker (after specifying the control
     * pin by calling [Buzzer].attach([buzzerPin]) ). You can specify the reference pitch for the buzzer (A4).
     *  
     * !! OTHER METHODS WON'T WORK IF THE BUZZER ISN'T ATTACHED !!
     */
    Buzzer(float referencePitch = 440);
    ~Buzzer();

    /**
     * attach():
     * ------------------------------------------------------------------------------------------------
     * Associates this Buzzer object with a buzzer connected to the specified pin.
     */
    void attach(uint8_t buzzerPin);

    /**
     * setReference():
     * ------------------------------------------------------------------------------------------------
     * Changes the reference pitch (A4) of this buzzer.
     */
    void setReference(float referencePitch);
    
    /**
     * playTone():
     * ------------------------------------------------------------------------------------------------
     * Plays the given frequency/note for the specified amount of time.
     */
    void playTone(float frequency, long duration = INDEFINITE);
    void playNote(Note note, long duration = INDEFINITE);

    /**
     * playSequence():
     * ------------------------------------------------------------------------------------------------
     * Plays the given sequence of frequencies/notes for the given durations specified by arrays.
     */
    void playSequence(float *frequencies, long *duration, int nFrequencies);
    void playSequence(Note *notes, long *duration, int nNotes);

    /**
     * isPlaying():
     * ------------------------------------------------------------------------------------------------
     * Returns true if the buzzer is still playing a tone.
     */
    bool isPlaying();

    /**
     * isPlaying():
     * ------------------------------------------------------------------------------------------------
     * Immediately stops the buzzer's sound.
     */
    void stop();

    void play(); // This method is for the timer interrupt, you do not need it.

private:

    uint8_t buzzerPin;
    
    int sequenceSize;
    volatile int currNote;
    volatile unsigned long playTimer;
    float referencePitch, **noteFrequencies;

    volatile int playState;

    float *currSequence;
    long *currDuration;

    void startTone(float frequency);
    void stopTone();
    
    void getNoteFrequencies();

    void getSequence(float *frequencies, long *duration, int nFrequencies);
    void getSequence(Note *notes, long *duration, int nNotes);

    void addBuzzer();
    void removeBuzzer();
    void enableTimer();
};

#endif