#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Util.h"
#include "Helper2D.h"
#include "Renderer3D.h"
#include "Mesh.h"

#include "Camera.h"
#include "constants.h"
#include "GameState.h"

#include "CardRender.h"
#include "ChipRender.h"
#include "TableRender.h"
#include "DeckRender.h"
#include "NoteBoardRender.h"
#include "AssimpLoader.h"

// Autor: Viktor Srbljin
// Opis: glavna logika igrice
// kako igrati se moze videti ulazenjem u igricu i citanjem note boarda (pravila igre)

// ANIM STATE  
static float chipHoverLift = 0.0f;
static float deckHoverLift = 0.0f;

struct ChipClickAnim {
    bool active = false;
    int stack = -1;
    float t = 0.0f;
    float dur = 0.60f;
    float spins = 3.0f;
} chipClick;

// ================== LOCAL HELPERS  ==================
static unsigned int createOverlayQuadVAO() {
    float quad[] = {
        -1.f,  1.f, 0.f, 1.f,
        -1.f, -1.f, 0.f, 0.f,
         1.f, -1.f, 1.f, 0.f,
         1.f,  1.f, 1.f, 1.f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

static float easeInOut(float u) {
    return u * u * (3.0f - 2.0f * u);
}

static glm::vec3 lerp3(const glm::vec3& a, const glm::vec3& b, float u) {
    return a + (b - a) * u;
}


static float stackEpsY(int i) { return 0.0020f * (float)i; }

static glm::vec3 cardSlotPos(const glm::vec3& base, int i, float spacingX) {
    return base + glm::vec3((float)i * spacingX, stackEpsY(i), 0.0f);
}

struct CardObj {
    CardRender r;
    std::string frontPath;

    glm::vec3 basePos{ 0 };
    glm::vec3 baseRot{ 0 };

    bool active = false;
    bool initialized = false;

    int rank = 0; // 1..13
    int suit = 0; // 0..3
};

// ================================ MAIN ================================
int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "3D Kostur", monitor, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK) { glfwTerminate(); return -1; }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GLFWcursor* cursorDefault = loadImageToCursor("Resources/hand_thin_point.png");
    GLFWcursor* cursorChip = loadImageToCursor("Resources/hand_thin_open.png");
    GLFWcursor* cursorDeck = loadImageToCursor("Resources/look_d.png");
    glfwSetCursor(window, cursorDefault);

    unsigned int overlayShader = createShader("texture.vert", "texture.frag");
    unsigned int overlayVAO = createOverlayQuadVAO();

    unsigned int texPlayerWins = loadImageToTexture("Resources/player_wins.png");
    glBindTexture(GL_TEXTURE_2D, texPlayerWins);
    setTextureParamsClamp();
    glGenerateMipmap(GL_TEXTURE_2D);

    unsigned int texDealerWins = loadImageToTexture("Resources/dealer_wins.png");
    glBindTexture(GL_TEXTURE_2D, texDealerWins);
    setTextureParamsClamp();
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    for (int d = 0; d < 10; d++) {
        std::string path = "Resources/" + std::to_string(d) + ".png";
        digitTextures[d] = loadImageToTexture(path.c_str());

        glBindTexture(GL_TEXTURE_2D, digitTextures[d]);
        setTextureParamsClamp();
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    texMyInfo = loadImageToTexture("Resources/my_info.png");
    glBindTexture(GL_TEXTURE_2D, texMyInfo);
    setTextureParamsClamp();
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int texBg = loadImageToTexture("Resources/casino_blur.png");
    glBindTexture(GL_TEXTURE_2D, texBg);
    setTextureParamsClamp();
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int texBet = loadImageToTexture("Resources/bet.png");
    glBindTexture(GL_TEXTURE_2D, texBet); setTextureParamsClamp(); glGenerateMipmap(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int texWon = loadImageToTexture("Resources/won.png");
    glBindTexture(GL_TEXTURE_2D, texWon); setTextureParamsClamp(); glGenerateMipmap(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int texLost = loadImageToTexture("Resources/lost.png");
    glBindTexture(GL_TEXTURE_2D, texLost); setTextureParamsClamp(); glGenerateMipmap(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, 0);

    // BET UI state
    static int gBet = 0;
    static int gWon = 0;
    static int gLost = 0;

    static bool gShowWinOverlay = false;
    static double gWinOverlayUntil = 0.0;
    static unsigned int gWinOverlayTex = 0;

    static bool gPendingWinOverlay = false;
    static double gWinOverlayStartAt = 0.0;

    static RoundResult gRoundResult = RoundResult::None;

    Renderer3D r3d;
    r3d.init();

    LoadedModel wine;

    try {
        wine = loadModelAssimp("Resources/wine-red.obj", "", { 180,20,20,255 });
    }
    catch (const std::exception& e) {
        std::cout << "MODEL LOAD ERROR: " << e.what() << std::endl;
    }

    TableRender table;
    table.init(128, 2.5f, 0.12f, "Resources/green_texture.png", 2.2f, 1.8f);
    table.pos = glm::vec3(0.0f);
    table.rot = glm::vec3(0.0f);
    table.scale = glm::vec3(1.0f);

    NoteBoardRender rulesBoard;
    rulesBoard.init("Resources/rules.png");
    rulesBoard.pos = glm::vec3(-3.0f, 0.9f, 0.0f);
    rulesBoard.rot = glm::vec3(glm::radians(-10.0f), glm::radians(35.0f), 0.0f);
    rulesBoard.boardW = 0.75f;
    rulesBoard.boardH = 1.20f;
    rulesBoard.boardT = 0.04f;

    // RNG
    std::random_device rd;
    std::mt19937 rng(rd());

    // ---------- Chips setup ----------
    const float tableTopY = Cfg::TABLE_TOP_Y;
    const float chipT = Cfg::CHIP_T;
    const float chipHalfT = chipT * 0.5f;

    const float stackSpacing = 0.45f;
    const float centerX = 0.0f;

    glm::vec3 stackBasePos[4];
    for (int i = 0; i < 4; i++) {
        float offset = (i - 1.5f) * stackSpacing;
        stackBasePos[i] = glm::vec3(centerX + offset, tableTopY + chipHalfT, 2.0f);
    }

    const char* chipTex[4] = {
        "Resources/chip_yellow.png",
        "Resources/chip_green.png",
        "Resources/chip_blue.png",
        "Resources/chip_red.png"
    };

    int stackCount[4];
    for (int i = 0; i < 4; i++) stackCount[i] = 1 + (rand() % 6);

    glm::u8vec4 sideCol[4] = {
        {170, 140,   0, 255},
        {  0, 120,  40, 255},
        { 20,  60, 160, 255},
        {160,  20,  20, 255},
    };

    std::vector<ChipRender> chips;
    chips.reserve(24);

    std::vector<glm::vec3> chipBasePos;
    chipBasePos.reserve(24);

    std::vector<glm::vec3> chipBaseRot;
    chipBaseRot.reserve(24);

    int stackStart[4] = { 0,0,0,0 };

    for (int s = 0; s < 4; s++) {
        stackStart[s] = (int)chips.size();
        for (int i = 0; i < stackCount[s]; i++) {
            ChipRender c;
            c.init(64, 0.18f, chipT, chipTex[s], sideCol[s]);
            c.pos = stackBasePos[s] + glm::vec3(0.0f, i * chipT, 0.0f);
            c.rot = glm::vec3(0.0f);
            c.scale = glm::vec3(1.0f);

            chips.push_back(c);
            chipBasePos.push_back(c.pos);
            chipBaseRot.push_back(c.rot);
        }
    }

    // Deck render
    DeckRender deck;
    deck.init(0.18f, "Resources/back_red_big.png");
    deck.pos = glm::vec3(-1.8f, 0.15f, 0.25f);
    deck.rot = glm::vec3(0.0f);
    deck.scale = glm::vec3(1.0f);

    // Hover top card 
    CardRender deckTopCard;
    deckTopCard.init(16, "Resources/back_red_big.png", "Resources/back_red_big.png");
    deckTopCard.scale = glm::vec3(1.0f);

    // Camera
    Camera cam;
    cam.pos = glm::vec3(0.0f, 3.5f, 4.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    cam.front = glm::normalize(target - cam.pos);
    cam.up = glm::vec3(0, 1, 0);
    cam.fov = Cfg::CAM_FOV;

    // Card positions
    const float cardY = tableTopY + 0.10f;
    glm::vec3 playerBase = glm::vec3(-0.35f, cardY, 1.30f);
    glm::vec3 dealerBase = glm::vec3(-0.35f, cardY, -0.60f);
    float cardSpacingX = 0.38f;

    glm::vec3 deckTopPos = deck.pos + glm::vec3(0.0f, 0.12f, 0.0f);

    // Cards container 
    std::vector<CardObj> cards; cards.reserve(64);
    std::vector<int> cardRanks; cardRanks.reserve(64); 
    std::vector<CardAnim> animQ; animQ.reserve(128);

    auto destroyAllCards = [&]() {
        for (auto& c : cards) {
            if (c.initialized) c.r.destroy();
        }
        cards.clear();
        cardRanks.clear();
        animQ.clear();
        };

    Deck52 deck52;
    deck52.resetAndShuffle(rng);

    TurnState game = TurnState::IdleNoDeal;
    bool initialDealQueued = false;

    std::vector<int> playerHand;
    std::vector<int> dealerHand;

    int dealerFaceDownIndex = -1;

    // round end timer
    bool pendingReset = false;
    double resetAtTime = 0.0;

    auto scheduleReset3s = [&](RoundResult rr) {
        gRoundResult = rr;

        // bet to won/lost 
        if (gBet > 0) {
            if (rr == RoundResult::PlayerWin) gWon += gBet;
            else if (rr == RoundResult::DealerWin) gLost += gBet;
            gBet = 0;
        }

        gWinOverlayTex = (rr == RoundResult::PlayerWin) ? texPlayerWins : texDealerWins;

        gPendingWinOverlay = (rr != RoundResult::None);
        gWinOverlayStartAt = glfwGetTime() + 2.0;
        gWinOverlayUntil = gWinOverlayStartAt + 3.0;

        gShowWinOverlay = false;

        pendingReset = true;
        resetAtTime = gWinOverlayUntil;
        game = TurnState::RoundOverWait;
        };

    auto resetRoundInstant = [&]() {
        destroyAllCards();
        deck52.resetAndShuffle(rng);

        playerHand.clear();
        dealerHand.clear();

        dealerFaceDownIndex = -1;
        game = TurnState::IdleNoDeal;
        initialDealQueued = false;

        pendingReset = false;
        resetAtTime = 0.0;
        };

    auto createCardObj = [&](const CardId& id, bool startFaceDown) -> int {
        CardObj o;
        o.frontPath = cardPath(id); 
        o.rank = id.rank;
        o.suit = id.suit;

        o.basePos = deckTopPos;
        o.baseRot = glm::vec3(startFaceDown ? Cfg::CARD_FACEDOWN_X : Cfg::CARD_FACEUP_X, 0.0f, 0.0f);

        o.active = false;
        o.initialized = false;

        cards.push_back(o);
        cardRanks.push_back(id.rank);

        return (int)cards.size() - 1;
        };

    auto ensureCardInitAndShow = [&](CardObj& co) {
        if (!co.initialized) {
            co.r.init(16, co.frontPath.c_str(), "Resources/back_red_big.png");
            co.r.scale = glm::vec3(1.0f);
            co.initialized = true;
        }
        co.active = true;
        };

    auto enqueueDealFaceUpRotateThenMove = [&](const glm::vec3& targetPos) -> int {
        if (deck52.empty()) deck52.resetAndShuffle(rng);
        CardId id = deck52.draw();

        int idx = createCardObj(id, true);

        CardAnim a;
        a.cardIndex = idx;
        a.type = CardAnimType::DealFaceUpRotateThenMove;
        a.startPos = deckTopPos;
        a.endPos = targetPos;
        a.startRot = glm::vec3(Cfg::CARD_FACEDOWN_X, 0.0f, 0.0f);
        a.endRot = glm::vec3(Cfg::CARD_FACEUP_X, 0.0f, 0.0f);
        animQ.push_back(a);
        return idx;
        };

    auto enqueueDealFaceDownMoveOnly = [&](const glm::vec3& targetPos) -> int {
        if (deck52.empty()) deck52.resetAndShuffle(rng);
        CardId id = deck52.draw();

        int idx = createCardObj(id, true);

        CardAnim a;
        a.cardIndex = idx;
        a.type = CardAnimType::DealFaceDownMoveOnly;
        a.startPos = deckTopPos;
        a.endPos = targetPos;
        a.startRot = glm::vec3(Cfg::CARD_FACEDOWN_X, 0.0f, 0.0f);
        a.endRot = a.startRot;
        animQ.push_back(a);
        return idx;
        };

    auto enqueueFlipInPlaceToFaceUp = [&](int cardIndex) {
        if (cardIndex < 0 || cardIndex >= (int)cards.size()) return;
        if (!cards[cardIndex].initialized) return;

        CardAnim a;
        a.cardIndex = cardIndex;
        a.type = CardAnimType::FlipInPlace;

        a.startPos = cards[cardIndex].r.pos;
        a.endPos = cards[cardIndex].r.pos;

        a.startRot = cards[cardIndex].r.rot;
        a.endRot = glm::vec3(Cfg::CARD_FACEUP_X, 0.0f, 0.0f);

        a.durLift = 0.0f;
        a.liftH = 0.0f;
        a.durRotate = 0.30f;
        a.durMove = 0.0f;

        animQ.push_back(a);
        };

    auto playerScore = [&]() -> int {
        return bestScoreAce1or10(playerHand, cardRanks);
        };
    auto dealerScore = [&]() -> int {
        return bestScoreAce1or10(dealerHand, cardRanks);
        };

    auto resolveIfNeeded = [&]() {
        int ps = playerScore();
        int ds = dealerScore();

        if (game == TurnState::PlayerTurn) {
            if (ps > 21) scheduleReset3s(RoundResult::DealerWin);
        }
        else if (game == TurnState::DealerTurn) {
            if (ds > 21) { scheduleReset3s(RoundResult::PlayerWin); return; }

            if (ds >= 17 || (int)dealerHand.size() >= 6) {
                if (ps > ds) scheduleReset3s(RoundResult::PlayerWin);
                else scheduleReset3s(RoundResult::DealerWin);
            }
        }
        };

    double prevTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        int ww, wh;
        glfwGetFramebufferSize(window, &ww, &wh);

        double nowT = glfwGetTime();
        float dt = (float)(nowT - prevTime);
        prevTime = nowT;

        updateCameraFPS(window, cam, dt);

        // timers
        if (pendingReset && nowT >= resetAtTime) {
            resetRoundInstant();
        }

        if (gPendingWinOverlay && !gShowWinOverlay && nowT >= gWinOverlayStartAt) {
            gShowWinOverlay = true;
            gPendingWinOverlay = false;
        }

        if (gShowWinOverlay && nowT >= gWinOverlayUntil) {
            gShowWinOverlay = false;
            gWinOverlayTex = 0;
        }

        bool busy = !animQ.empty();
        bool inputLocked = (gShowWinOverlay || pendingReset || game == TurnState::RoundOverWait);

        // Hover detection
        bool hoverDeck = hoverWorldPointPx(window, cam, deck.pos, ww, wh, Cfg::HOVER_DECK_RADIUS_PX);

        int hoveredStack = -1;
        for (int s = 0; s < 4; s++) {
            glm::vec3 topOfStack = stackBasePos[s] + glm::vec3(0.0f, (stackCount[s] - 1) * chipT, 0.0f);
            if (hoverWorldPointPx(window, cam, topOfStack, ww, wh, Cfg::HOVER_CHIP_RADIUS_PX)) {
                hoveredStack = s;
                break;
            }
        }
        bool hoverChips = (hoveredStack != -1);

        // Smooth lifts
        chipHoverLift = expApproach(chipHoverLift, hoverChips ? 0.06f : 0.0f, 14.0f, dt);
        deckHoverLift = expApproach(deckHoverLift, hoverDeck ? 0.20f : 0.0f, 14.0f, dt);

        // Chip click
        bool clickChip = (!inputLocked) && (hoveredStack != -1) && mousePressedOnce(window, GLFW_MOUSE_BUTTON_LEFT);

        if (clickChip && !chipClick.active) {
            if (game == TurnState::IdleNoDeal && !busy && !pendingReset) {
                gBet += Cfg::CHIP_VALUE[hoveredStack];

                chipClick.active = true;
                chipClick.stack = hoveredStack;
                chipClick.t = 0.0f;
            }
        }
        if (chipClick.active) {
            chipClick.t += dt;
            if (chipClick.t >= chipClick.dur) {
                chipClick.active = false;
                chipClick.stack = -1;
                chipClick.t = 0.0f;
            }
        }

        // GAME INPUT 
        bool clickDeck = (!inputLocked) && hoverDeck && mousePressedOnce(window, GLFW_MOUSE_BUTTON_LEFT);
        bool pressSpace = (!inputLocked) && keyPressedOnce(window, GLFW_KEY_SPACE);

        if (keyPressedOnce(window, GLFW_KEY_Z)) gDepthEnabled = !gDepthEnabled;
        if (keyPressedOnce(window, GLFW_KEY_X)) gCullEnabled = !gCullEnabled;

        if (keyPressedOnce(window, GLFW_KEY_N)) {
            gLightEnabled = !gLightEnabled;
        }

        bool suppressDeckHoverCard = inputLocked || busy || clickDeck;

        if (clickDeck && !busy && game != TurnState::RoundOverWait) {
            if (game == TurnState::IdleNoDeal) {
                if (gBet > 0) {
                    resetRoundInstant();

                    int p0 = enqueueDealFaceUpRotateThenMove(cardSlotPos(playerBase, (int)playerHand.size(), cardSpacingX));
                    playerHand.push_back(p0);

                    int p1 = enqueueDealFaceUpRotateThenMove(cardSlotPos(playerBase, (int)playerHand.size(), cardSpacingX));
                    playerHand.push_back(p1);

                    int d0 = enqueueDealFaceDownMoveOnly(cardSlotPos(dealerBase, (int)dealerHand.size(), cardSpacingX));
                    dealerFaceDownIndex = d0;
                    dealerHand.push_back(d0);

                    int d1 = enqueueDealFaceUpRotateThenMove(cardSlotPos(dealerBase, (int)dealerHand.size(), cardSpacingX));
                    dealerHand.push_back(d1);

                    initialDealQueued = true;
                    game = TurnState::IdleNoDeal; // prebaci na PlayerTurn kad se animacije završe
                }
            }
            else if (game == TurnState::PlayerTurn) {
                if ((int)playerHand.size() < 6) {
                    int p = enqueueDealFaceUpRotateThenMove(cardSlotPos(playerBase, (int)playerHand.size(), cardSpacingX));
                    playerHand.push_back(p);
                }
            }
        }

        if (pressSpace && !busy && game != TurnState::RoundOverWait) {
            if (game == TurnState::PlayerTurn) {
                game = TurnState::DealerTurn;
                enqueueFlipInPlaceToFaceUp(dealerFaceDownIndex);
            }
        }

        //  Anim queue update
        if (!animQ.empty()) {
            CardAnim& a = animQ.front();
            CardObj& co = cards[a.cardIndex];

            ensureCardInitAndShow(co);
            a.t += dt;

            if (a.type == CardAnimType::FlipInPlace) {
                float u = (a.durRotate > 0.0f) ? (a.t / a.durRotate) : 1.0f;
                if (u > 1.0f) u = 1.0f;
                float e = easeInOut(u);

                co.r.pos = a.startPos;
                co.r.rot = lerp3(a.startRot, a.endRot, e);

                if (u >= 1.0f) {
                    co.r.pos = a.endPos;
                    co.r.rot = a.endRot;
                    animQ.erase(animQ.begin());
                }
            }
            else if (a.type == CardAnimType::DealFaceUpRotateThenMove) {
                if (a.t < a.durLift) {
                    float u = a.t / a.durLift;
                    float e = easeInOut(u);
                    co.r.rot = a.startRot;
                    co.r.pos = a.startPos + glm::vec3(0.0f, a.liftH * e, 0.0f);
                }
                else if (a.t < a.durLift + a.durRotate) {
                    float tr = a.t - a.durLift;
                    float u = tr / a.durRotate;
                    float e = easeInOut(u);
                    co.r.pos = a.startPos + glm::vec3(0.0f, a.liftH, 0.0f);
                    co.r.rot = lerp3(a.startRot, a.endRot, e);
                }
                else {
                    float tm = a.t - (a.durLift + a.durRotate);
                    float u = (a.durMove > 0.0f) ? (tm / a.durMove) : 1.0f;
                    if (u > 1.0f) u = 1.0f;
                    float e = easeInOut(u);

                    co.r.rot = a.endRot;

                    glm::vec3 startLifted = a.startPos + glm::vec3(0.0f, a.liftH, 0.0f);
                    glm::vec3 p = lerp3(startLifted, a.endPos, e);

                    float arc = std::sin(3.1415926f * e) * 0.12f;
                    p.y += arc;

                    co.r.pos = p;

                    if (u >= 1.0f) {
                        co.r.pos = a.endPos;
                        animQ.erase(animQ.begin());
                    }
                }
            }
            else if (a.type == CardAnimType::DealFaceDownMoveOnly) {
                float u = (a.durMove > 0.0f) ? (a.t / a.durMove) : 1.0f;
                if (u > 1.0f) u = 1.0f;
                float e = easeInOut(u);

                co.r.rot = a.startRot;
                glm::vec3 p = lerp3(a.startPos, a.endPos, e);
                float arc = std::sin(3.1415926f * e) * 0.18f;
                p.y += arc;
                co.r.pos = p;

                if (u >= 1.0f) {
                    co.r.pos = a.endPos;
                    animQ.erase(animQ.begin());
                }
            }

            co.basePos = co.r.pos;
            co.baseRot = co.r.rot;
        }
        else {
            if (initialDealQueued && game == TurnState::IdleNoDeal) {
                game = TurnState::PlayerTurn;
                initialDealQueued = false;
                resolveIfNeeded();
            }

            if (game == TurnState::DealerTurn && game != TurnState::RoundOverWait) {
                int ds = dealerScore();
                if (ds < 17 && (int)dealerHand.size() < 6) {
                    int d = enqueueDealFaceUpRotateThenMove(cardSlotPos(dealerBase, (int)dealerHand.size(), cardSpacingX));
                    dealerHand.push_back(d);
                }
                else {
                    resolveIfNeeded();
                }
            }

            if (game == TurnState::PlayerTurn && game != TurnState::RoundOverWait) {
                resolveIfNeeded();
            }
        }

        // Cursor 
        GLFWcursor* cur = cursorDefault;
        if (!inputLocked) {
            if (hoverDeck) cur = cursorDeck;
            else if (hoverChips) cur = cursorChip;
        }

        static GLFWcursor* lastCur = nullptr;
        if (cur != lastCur) {
            glfwSetCursor(window, cur);
            lastCur = cur;
        }

        if (gDepthEnabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);

        if (gCullEnabled) glEnable(GL_CULL_FACE);
        else glDisable(GL_CULL_FACE);

        // Render 
        glViewport(0, 0, ww, wh);
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Background
        glDisable(GL_DEPTH_TEST);
        glUseProgram(overlayShader);
        glUniform1f(glGetUniformLocation(overlayShader, "uAlphaMul"), 1.0f);
        drawQuad(overlayShader, texBg, 0.0f, 0.0f, 1.0f, 1.0f, overlayVAO, false);

        if (gDepthEnabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);

        r3d.begin(cam, ww, wh);
        table.draw(r3d, cam);
        rulesBoard.draw(r3d, cam);

        glm::mat4 Mw(1.0f);
        Mw = glm::translate(Mw, glm::vec3(1.5f, Cfg::TABLE_TOP_Y + 0.02f, -0.7f));
        Mw = glm::scale(Mw, glm::vec3(2.2f));   
        Mw = glm::rotate(Mw, glm::radians(0.0f), glm::vec3(1,0,0));

        r3d.draw(wine.mesh, wine.tex, Mw, cam);

        // ---- chips ----
        glDisable(GL_CULL_FACE);

        for (size_t i = 0; i < chips.size(); i++) {
            chips[i].pos = chipBasePos[i];
            chips[i].rot = chipBaseRot[i];
        }

        if (hoveredStack != -1 && !chipClick.active) {
            int topIdx = stackStart[hoveredStack] + (stackCount[hoveredStack] - 1);
            chips[topIdx].pos.y += chipHoverLift;
        }

        if (chipClick.active && chipClick.stack != -1) {
            int topIdx = stackStart[chipClick.stack] + (stackCount[chipClick.stack] - 1);

            float u = chipClick.t / chipClick.dur;
            u = glm::clamp(u, 0.0f, 1.0f);

            float lift = std::sin(3.1415926f * u);
            chips[topIdx].pos.y += 0.28f * lift;

            float spin = u * chipClick.spins * 6.2831853f;
            chips[topIdx].rot.y += spin;
            chips[topIdx].rot.z += 0.35f * lift;
        }

        for (auto& c : chips) c.draw(r3d, cam);

        glEnable(GL_CULL_FACE);
 
        deck.draw(r3d, cam);

        if (!suppressDeckHoverCard && deckHoverLift > 0.001f && game != TurnState::RoundOverWait) {
            deckTopCard.pos = deck.pos + glm::vec3(0.0f, 0.12f + deckHoverLift, 0.0f);
            deckTopCard.rot = glm::vec3(Cfg::CARD_FACEDOWN_X, 0.0f, 0.0f);

            glDisable(GL_CULL_FACE);
            deckTopCard.draw(r3d, cam);
            glEnable(GL_CULL_FACE);
        }

        glDisable(GL_CULL_FACE);
        for (auto& c : cards) {
            if (!c.active || !c.initialized) continue;
            c.r.draw(r3d, cam);
        }
        glEnable(GL_CULL_FACE);

        // ---- 2D overlay ----
        glDisable(GL_DEPTH_TEST);
        glUseProgram(overlayShader);
        glUniform1f(glGetUniformLocation(overlayShader, "uAlphaMul"), 0.5f);
        drawQuad(overlayShader, texMyInfo, 0.90f, 0.95f, 0.10f, 0.10f, overlayVAO, false);
        glUniform1f(glGetUniformLocation(overlayShader, "uAlphaMul"), 1.0f);

        drawQuad(overlayShader, texBet, -0.82f, 0.90f, 0.22f, 0.08f, overlayVAO, false);
        drawQuad(overlayShader, texWon, -0.82f, 0.80f, 0.22f, 0.08f, overlayVAO, false);
        drawQuad(overlayShader, texLost, -0.82f, 0.70f, 0.22f, 0.08f, overlayVAO, false);

        drawNumber(overlayShader, gBet, -0.55f, 0.90f, 0.035f, overlayVAO);
        drawNumber(overlayShader, gWon, -0.55f, 0.80f, 0.035f, overlayVAO);
        drawNumber(overlayShader, gLost, -0.55f, 0.70f, 0.035f, overlayVAO);

        if (gShowWinOverlay && gWinOverlayTex != 0) {
            glDisable(GL_DEPTH_TEST);
            glUseProgram(overlayShader);

            glUniform1i(glGetUniformLocation(overlayShader, "uUseColor"), 1);
            glUniform4f(glGetUniformLocation(overlayShader, "uColorFill"), 0.0f, 0.0f, 0.0f, 0.65f);
            drawQuad(overlayShader, texBg, 0.0f, 0.0f, 1.0f, 1.0f, overlayVAO, false);

            glUniform1i(glGetUniformLocation(overlayShader, "uUseColor"), 0);
            glUniform1f(glGetUniformLocation(overlayShader, "uAlphaMul"), 1.0f);

            float sx = 0.55f;
            float sy = 0.20f;
            drawQuad(overlayShader, gWinOverlayTex, 0.0f, 0.05f, sx, sy, overlayVAO, false);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        limitFPS(Cfg::TARGET_FPS);
    }

    // cleanup 
    table.destroy();
    rulesBoard.destroy();
    for (auto& c : chips) c.destroy();
    deck.destroy();
    deckTopCard.destroy();
    destroyAllCards();

    if (cursorDefault) glfwDestroyCursor(cursorDefault);
    if (cursorChip)    glfwDestroyCursor(cursorChip);
    if (cursorDeck)    glfwDestroyCursor(cursorDeck);

    glfwTerminate();
    return 0;
}
