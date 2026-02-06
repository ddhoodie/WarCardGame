#pragma once
#include <vector>
#include <string>
#include <random>
#include <glm/glm.hpp>

struct Transform {
    glm::vec3 pos{ 0,0,0 };
    glm::vec3 rot{ 0,0,0 };   // Euler (radians) - jednostavno za start
    glm::vec3 scale{ 1,1,1 };
};

struct CardEntity {
    int cardID = -1;            // suit*13 + (rank-1)
    bool occupied = false;

    // anim states (reuse ideju iz 2D)
    float dealT = 0.0f;         // 0..1 kretanje od deck -> target
    float flipT = 0.0f;         // 0..1 rotacija 0..PI
    bool flipping = false;
    bool faceUp = false;

    bool selected = false;
    float selectT = 0.0f;       // 0..1 lift

    Transform tr;
};

extern bool gDepthEnabled;
extern bool gCullEnabled;
extern bool gLightEnabled;

// Scene transforms
extern Transform gTableTr;
extern Transform gDeckRedTr;
extern Transform gDeckBlueTr;


constexpr int HAND_SIZE = 5;
extern CardEntity hand3D[HAND_SIZE];
extern CardEntity hand3D_2[HAND_SIZE];

// 2D overlay textures (reuse)
extern unsigned int texMyInfo;
extern unsigned int texTable;
extern unsigned int texDeckRed;
extern unsigned int texDeckBlue;

// card textures reuse (ako ostaje)
extern unsigned int cardFaceTextures[4][14];

enum class TurnState { IdleNoDeal, PlayerTurn, DealerTurn, RoundOverWait };
enum class RoundResult { None, PlayerWin, DealerWin };

enum class CardAnimType { DealFaceUpRotateThenMove, DealFaceDownMoveOnly, FlipInPlace };

struct CardAnim {
    int cardIndex = -1;
    CardAnimType type{};
    glm::vec3 startPos{ 0 }, endPos{ 0 };
    glm::vec3 startRot{ 0 }, endRot{ 0 };
    float t = 0.0f;
    float durLift = 0.12f;
    float liftH = 0.16f;
    float durRotate = 0.30f;
    float durMove = 0.35f;
};

struct CardId { int suit; int rank; };
std::string cardPath(const CardId& c);

struct Deck52 {
    std::vector<CardId> order;
    int top = 0;
    void resetAndShuffle(std::mt19937& rng);
    bool empty() const;
    CardId draw();
};

int bestScoreAce1or10(const std::vector<int>& handIdx, const std::vector<int>& cardRanks);
