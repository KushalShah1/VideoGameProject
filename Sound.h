// Sound.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// November 17, 2014

#ifndef __Sound_h
#define __Sound_h
enum soundState{death,coinSound,background,powerup};
void Sound_Init(void);
void Sound_Play(void);
void setSoundState(enum soundState x);
#endif /* __Sound_h */
