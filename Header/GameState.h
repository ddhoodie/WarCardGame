#pragma once

#include <vector>
#include "CardTypes.h"
#include "Constants.h"

// GLOBAL SHADER / VAO
extern unsigned int globalShader;
extern unsigned int globalVAO;

// TEXTURE ID-ovi
extern unsigned int texBG;
extern unsigned int texDeckRed;
extern unsigned int texDeckBlue;
extern unsigned int texRedWins;
extern unsigned int texBlueWins;
extern unsigned int texRespect;
extern unsigned int texRageBtn;

extern unsigned int digitTextures[10];
extern unsigned int cardFaceTextures[4][14];

extern unsigned int texRespectPopup;
extern unsigned int texRageAnim[3];

// state of deck anim
extern float lifted;
extern float lifted2;

// position of cards in hands
extern float handCardX[HAND_SIZE];
extern float moveToHand[HAND_SIZE];
extern int   cardsInHand;

extern float handCardX2[HAND_SIZE];
extern float moveToHand2[HAND_SIZE];
extern int   cardsInHand2;

// hands, decks
extern HandCard hand[HAND_SIZE];
extern HandCard hand2[HAND_SIZE];

extern std::vector<Card> deck;
extern std::vector<Card> deck2;

// played cards
extern PlayedCard playedRed;
extern PlayedCard playedBlue;
extern bool hasPlayedRed;
extern bool hasPlayedBlue;

// card selection
extern int  selectedIndex1;
extern int  selectedIndex2;
extern bool roundLocked;
extern float resolveTimer;

// discards
extern std::vector<Card> discardRed;
extern std::vector<Card> discardBlue;

// state of round
extern RoundState roundState;
extern float collectTimer;
extern int   collectPhase;
extern float collectFlip;

extern bool gameOver;
extern bool redWinner;

// end game floating anims
extern FloatingAnim respectAnim;
extern FloatingAnim rageAnim;

// fps limit
extern double lastTimeForRefresh;
extern const double FPS;

// funcs
void limitFPS();
void generateDecks();
void startRespectAnim();
void startRageAnim(bool redWinner);
void updateFloatingAnim(FloatingAnim& a, float speed);
void initHands();
void resetGame();
