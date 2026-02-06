#pragma once
#pragma once
#include "GameState.h"

void startRound();

void updateRoundReveal(float mx, float my);

void updateRoundWait(float mx, float my);

void updateRoundCollect(float mx, float my);

void updateRoundEndgame(float mx, float my, int ww, int wh);