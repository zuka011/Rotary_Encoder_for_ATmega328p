#include "Buzzer.h"

#define N_LETTERS 12
#define START_OCTAVE 2  
#define END_OCTAVE 8
#define N_OCTAVES (END_OCTAVE - START_OCTAVE)

#define SEMITONE_RATIO 1.059463094359295
#define A4_C2_RATIO 6.726800183458187
#define USECONDS_IN_SECOND 1e6

#define CLOCK_FREQUENCY 16e6
#define TIMER1_PRESCALER 1
#define TIMER1_MAX 65535
#define TIMER1_MIN 256
#define MAX_BUZZERS 2

static volatile Buzzer* callback_buzzers[MAX_BUZZERS];
static volatile int last_buzzer = 0;

String Note::toString() {

    String stringRep;
    switch(name) {
        case C: stringRep += "C"; break;
        case Db: stringRep += "Db"; break;
        case D: stringRep += "D"; break;
        case Eb: stringRep += "Eb"; break;
        case E: stringRep += "E"; break;
        case F: stringRep += "F"; break;
        case Gb: stringRep += "Gb"; break;
        case G: stringRep += "G"; break;
        case Ab: stringRep += "Ab"; break;
        case A: stringRep += "A"; break;
        case Bb: stringRep += "Bb"; break;
        case B: stringRep += "B"; break;
    }
    return stringRep + String(octave);
}

bool operator==(const Note & note1, const Note & note2) {
    return note1.name == note2.name && note1.octave == note2.octave;
}

Buzzer::Buzzer(float referencePitch) {

    this->referencePitch = referencePitch;

    currSequence = NULL;
    currDuration = NULL;
    noteFrequencies = NULL;

    sequenceSize = 0;
    currNote = 0;
    playState = 0;

    getNoteFrequencies();
}

Buzzer::~Buzzer() {

    removeBuzzer();
    
    delete[] currSequence;
    
    for(int i = 0; i < N_LETTERS; i++) delete[] noteFrequencies[i];
    delete[] noteFrequencies;
}

void Buzzer::attach(uint8_t buzzerPin) {

    this->buzzerPin = buzzerPin;

    addBuzzer();
    enableTimer();
}

void Buzzer::setReference(float referencePitch) {

    this->referencePitch = referencePitch;
    getNoteFrequencies();
}

void Buzzer::playTone(float frequency, long duration = INDEFINITE) {
    playSequence(&frequency, &duration, 1);
}

void Buzzer::playNote(Note note, long duration = INDEFINITE) {
    playSequence(&note, &duration, 1);
}

void Buzzer::playSequence(float *frequencies, long *duration, int nFrequencies) {

    getSequence(frequencies, duration, nFrequencies);
    sequenceSize = nFrequencies;
    currNote = 0;
    playState = 0;
}

void Buzzer::playSequence(Note *notes, long *duration, int nNotes) {

    getSequence(notes, duration, nNotes);
    sequenceSize = nNotes;
    currNote = 0;
    playState = 0;
}

bool Buzzer::isPlaying() { 
    return currNote < sequenceSize;
}

void Buzzer::stop() {

    currNote = sequenceSize;
    stopTone();
}

void Buzzer::play() {

    switch(playState) {

        case 0:

            if(currSequence[currNote] == REST_NOTE) stopTone();
            else startTone(currSequence[currNote]);
            playState++;
        case 1:

            if(currNote == sequenceSize) playState++;
            else if(currDuration[currNote] == INDEFINITE || millis() - playTimer < currDuration[currNote]) return;
            else {

                playTimer = millis();
                currNote++;
                playState = 0;
                return;
            }
        case 2: stopTone();
    }
}

void Buzzer::startTone(float frequency) {

    pinMode(buzzerPin, OUTPUT);

    unsigned int newICRValue = CLOCK_FREQUENCY / (TIMER1_PRESCALER * frequency);
    
    if(newICRValue > TIMER1_MAX) newICRValue = TIMER1_MAX;
    else if(newICRValue < TIMER1_MIN) newICRValue = TIMER1_MIN;
    else ICR1 = newICRValue;
    
    switch(buzzerPin) {
        case 9: OCR1A = ICR1 / 2; break;
        case 10: OCR1B = ICR1 / 2; break;
    }
    TCNT1 = 0;
    playTimer = millis();
}

void Buzzer::stopTone() {

    ICR1 = TIMER1_MAX;
    pinMode(buzzerPin, INPUT);
}

void Buzzer::getNoteFrequencies() {
     
    if(noteFrequencies != NULL) {

        for(int i = 0; i < N_LETTERS; i++) delete[] noteFrequencies[i];
        delete[] noteFrequencies;
    }

    noteFrequencies = new float*[N_LETTERS];
    for(int i = 0; i < N_LETTERS; i++) noteFrequencies[i] = new float[N_OCTAVES];

    float pitchC2 = referencePitch / A4_C2_RATIO;

    for(int j = START_OCTAVE; j < END_OCTAVE; j++) {
        for(int i = C; i < N_LETTERS; i++) noteFrequencies[i][j - START_OCTAVE] = pitchC2 * pow(SEMITONE_RATIO, i + j*N_LETTERS);  
    }
}

void Buzzer::getSequence(float *frequencies, long *duration, int nFrequencies) {

    removeBuzzer();

    if(currSequence != NULL) delete[] currSequence;
    currSequence = new float[nFrequencies];

    if(currDuration != NULL) delete[] currDuration;
    currDuration = new long[nFrequencies];

    for(int i = 0; i < nFrequencies; i++) {
      
        currSequence[i] = frequencies[i];
        currDuration[i] = duration[i];
    }

    addBuzzer();
}

void Buzzer::getSequence(Note *notes, long *duration, int nNotes) {

    removeBuzzer();

    if(currSequence != NULL) delete[] currSequence;
    currSequence = new float[nNotes];

    if(currDuration != NULL) delete[] currDuration;
    currDuration = new long[nNotes];

    for(int i = 0; i < nNotes; i++) {
        
        if(notes[i] == REST) currSequence[i] = REST_NOTE;
        else if(notes[i].octave >= END_OCTAVE) currSequence[i] = noteFrequencies[notes[i].name][N_OCTAVES - 1];
        else if(notes[i].octave < START_OCTAVE) currSequence[i] = noteFrequencies[notes[i].name][0];
        else currSequence[i] = noteFrequencies[notes[i].name][notes[i].octave - START_OCTAVE];

        currDuration[i] = duration[i];
    }

    addBuzzer();
}

void Buzzer::addBuzzer() {

    if(last_buzzer == MAX_BUZZERS) return;
    for (int i = 0; i < last_buzzer; i++) if(callback_buzzers[i] == this) return;

    callback_buzzers[last_buzzer++] = this;
}

void Buzzer::removeBuzzer() {

    int index = -1;
    for (int i = 0; i < last_buzzer; i++) {
        
        if(callback_buzzers[i] == this) {

            index = i;
            break;
        }
    }

    if(index != -1)  {

        for(int i = index + 1; i < last_buzzer; i++) callback_buzzers[i - 1] = callback_buzzers[i]; 
        last_buzzer--;
    }
}

void Buzzer::enableTimer() {

    TCCR1A = _BV(WGM11);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);

    switch (buzzerPin) {

        case 9: TCCR1A |= _BV(COM1A1); break;
        case 10: TCCR1A |= _BV(COM1B1); break;
    }
    ICR1 = TIMER1_MAX;
    TIMSK1 |= _BV(ICIE1);
}

ISR(TIMER1_CAPT_vect) {

    static const int DELAY = 50;
    static volatile unsigned long timer = 0;

    if(millis() - timer < DELAY) return; 
    timer = millis();
    
    for(int i = 0; i < last_buzzer; i++) callback_buzzers[i]->play();
}