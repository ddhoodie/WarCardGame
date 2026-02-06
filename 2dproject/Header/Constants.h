#pragma once

// card dimensions
constexpr float CARD_WIDTH = 100.0f;
constexpr float CARD_HEIGHT = 250.0f;

// scaling
constexpr float SCALE_Y = 0.25f;
constexpr float SCALE_X = SCALE_Y * (CARD_WIDTH / CARD_HEIGHT);

// deck position
constexpr float DECK_X = 1.0f - SCALE_X - 0.05f;
constexpr float DECK_Y = -1.0f + SCALE_Y + 0.05f;

constexpr float DECK_X2 = -1.0f + SCALE_X + 0.05f;
constexpr float DECK_Y2 = 1.0f - SCALE_Y - 0.05f;

// hover effect
constexpr float HOVER_OFFSET_X = -0.03f;
constexpr float HOVER_OFFSET_Y = 0.05f;
constexpr float HOVER_SCALE_MUL = 1.04f;

// anims
constexpr float ANIM_SPEED = 1.2f;
constexpr float MOVE_SPEED = 0.3f;

// hand
constexpr int HAND_SIZE = 5;
constexpr float HAND_Y = -0.85f;
constexpr float HAND_Y2 = 0.85f;

// speeds
constexpr float FOLD_SPEED = 0.02f;
constexpr float PLAY_SPEED = 0.02f;
constexpr float FLIP_SPEED = 0.05f;
constexpr float SELECT_LIFT_SPEED = 0.05f;
constexpr float COLLECT_SPEED = 0.01f;
constexpr float RAGE_ANIM_SPEED = 0.01f;   
constexpr float RESPECT_ANIM_SPEED = 0.022f; 

enum RoundState {
    ROUND_IDLE,
    ROUND_REVEALING,
    ROUND_WAIT,
    ROUND_COLLECTING,
    ROUND_ENDGAME
};