#pragma once
#include <vector>
#include <GL/glew.h>

// this could all be made more pretty but didnt think it trough to be honest

// hard in deck
struct Card {
    int suit;   
    int rank;   
};

// card in hand
struct HandCard {
    int cardID;
    bool occupied = false;
    bool isFlipped = false;
    float flipProgress = 0.0f;
    bool flipping = false;

    float cornerFold = 0.0f;
    bool folding = false;

    bool playing = false;
    float playProgress = 0.0f;
    float startX = 0.0f;
    float startY = 0.0f;

    bool selected = false;
    float selectLift = 0.0f;
};

// card on table 
struct PlayedCard {
    unsigned int tex;
    float startX, startY;
    float progress = 0.0f;
    bool flipped = false;
    float flipProgress = 0.0f;
    bool flipping = false;

    int rank = 0;
	int suit = 0;

    float tableX = 0.0f;
    float tableY = 0.0f;

    float discardX = 0.0f;
    float discardY = 0.0f;
    unsigned int discardTex = 0;
};

// floating anim struc at the end of the game
struct FloatingAnim {
    unsigned int tex = 0;
    float x = 0;
    float y = 0;
    float scale = 0.15f;

    float targetX = 0;
    float targetY = 0;

    float progress = 0.0f;
    bool active = false;
};
