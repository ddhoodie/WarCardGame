#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <algorithm>
#include <iostream>

#include "../Header/Util.h"
#include "../Header/Helper.h"
#include "../Header/CardTypes.h"
#include "../Header/Constants.h"
#include "../Header/GameState.h"
#include "../Header/GameLogic.h"

// Main file 
// Lots of functions for handling stuff here that should be somewhere else but didn't think the project logic through that well.
// Go ctrl + f main() to so you can start from there.
// Viktor Srbljin SV63/2022

void handleRightClick(float mx, float my)
{
    static bool pressed = false;
    GLFWwindow* win = glfwGetCurrentContext();

    bool click = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    if (click && !pressed)
    {
        pressed = true;
        for (int i = 0; i < HAND_SIZE; i++)
        {
            if (!hand[i].occupied) continue;

            float t = easeOutCubic(moveToHand[i]);
            float cx = DECK_X * (1 - t) + handCardX[i] * t;
            float cy = DECK_Y * (1 - t) + HAND_Y * t;

            if (isHover(mx, my, cx, cy, SCALE_X, SCALE_Y))
                hand[i].folding = !hand[i].folding;
        }
    }

    if (!click) pressed = false;
}

void updateCornerFold()
{
    float speed = 0.02f;
    for (int i = 0; i < HAND_SIZE; i++)
    {
        float& f = hand[i].cornerFold;
        f += (hand[i].folding ? speed : -speed);
        if (f < 0) f = 0;
        if (f > 1) f = 1;
    }
}

void selectCard(int index, HandCard hand[], int& selectedIndex)
{
    if (selectedIndex == index)
    {
        hand[index].selected = false;
        selectedIndex = -1;
        return;
    }

    for (int i = 0; i < HAND_SIZE; i++)
        hand[i].selected = false;

    hand[index].selected = true;
    selectedIndex = index;
}

void updateSelection()
{
    // PLAYER RED
    for (int i = 0; i < HAND_SIZE; i++)
    {
        if (!hand[i].occupied) continue;
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_1 + i) == GLFW_PRESS)
            selectCard(i, hand, selectedIndex1);
    }

    // PLAYER BLUE
    for (int i = 0; i < HAND_SIZE; i++)
    {
        if (!hand2[i].occupied) continue;

        int key = (i < 4) ? GLFW_KEY_6 + i : GLFW_KEY_0;
        if (glfwGetKey(glfwGetCurrentContext(), key) == GLFW_PRESS)
            selectCard(i, hand2, selectedIndex2);
    }
}

void takeCardFromDeck(std::vector<Card>& deck, HandCard hand[],
    float moveArr[], int& cardsInHand)
{
    Card c = deck.back(); deck.pop_back();
    for (int i = 0; i < HAND_SIZE; i++)
        if (!hand[i].occupied) {
            hand[i].occupied = true;
            hand[i].cardID = c.suit * 13 + (c.rank - 1);
            hand[i].isFlipped = false;
            hand[i].flipProgress = 0;
            moveArr[i] = 0;
            cardsInHand++;
            return;
        }
}

void tryFlipCard(float mx, float my)
{
    // PLAYER RED
    for (int i = 0; i < HAND_SIZE; i++)
    {
        if (!hand[i].occupied) continue;

        float t = easeOutCubic(moveToHand[i]);
        float cx = DECK_X * (1 - t) + handCardX[i] * t;
        float cy = DECK_Y * (1 - t) + HAND_Y * t;

        if (isHover(mx, my, cx, cy, SCALE_X, SCALE_Y))
        {
            hand[i].isFlipped = !hand[i].isFlipped;
            hand[i].flipping = true;
            return;
        }
    }

    // PLAYER BLUE
    for (int i = 0; i < HAND_SIZE; i++)
    {
        if (!hand2[i].occupied) continue;

        float t = easeOutCubic(moveToHand2[i]);
        float cx = DECK_X2 * (1 - t) + handCardX2[i] * t;
        float cy = DECK_Y2 * (1 - t) + HAND_Y2 * t;

        if (isHover(mx, my, cx, cy, SCALE_X, SCALE_Y))
        {
            hand2[i].isFlipped = !hand2[i].isFlipped;
            hand2[i].flipping = true;
            return;
        }
    }
}

void handleLeftClick(float mx, float my, bool hoveringDeck1, bool hoveringDeck2)
{
    static bool pressed = false;

    GLFWwindow* window = glfwGetCurrentContext();

    bool click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (click && !pressed)
    {
        pressed = true;

        if (hoveringDeck1 && cardsInHand < HAND_SIZE && !deck.empty())
            takeCardFromDeck(deck, hand, moveToHand, cardsInHand);

        else if (hoveringDeck2 && cardsInHand2 < HAND_SIZE && !deck2.empty())
            takeCardFromDeck(deck2, hand2, moveToHand2, cardsInHand2);

        else
            tryFlipCard(mx, my);
    }
    if (!click) pressed = false;
}

float updateLift(bool hovering, float lifted)
{
    if (hovering)
        lifted += ANIM_SPEED * 0.016f;
    else
        lifted -= ANIM_SPEED * 0.016f;

    if (lifted < 0.0f) lifted = 0.0f;
    if (lifted > 1.0f) lifted = 1.0f;

    return lifted;
}

void renderDeckHover(
    unsigned int tex,
    float deckX, float deckY,
    float lifted
)
{
    drawQuad(globalShader, tex, deckX, deckY, SCALE_X, SCALE_Y, globalVAO);

    if (lifted > 0.0f)
    {
        float LerpX = HOVER_OFFSET_X * lifted;
        float LerpY = HOVER_OFFSET_Y * lifted;
        float LerpScale = 1.0f + (HOVER_SCALE_MUL - 1.0f) * lifted;

        drawQuad(globalShader, tex,
            deckX + LerpX + 0.005f,
            deckY + LerpY - 0.01f,
            SCALE_X * LerpScale * 1.01f,
            SCALE_Y * LerpScale * 1.01f,
            globalVAO,
            true
        );

        drawQuad(globalShader, tex,
            deckX + LerpX,
            deckY + LerpY,
            SCALE_X * LerpScale,
            SCALE_Y * LerpScale,
            globalVAO,
            false
        );
    }
}

void updateFlip(HandCard& c)
{
    if (!c.flipping) return;

    if (c.isFlipped)
    {
        c.flipProgress += FLIP_SPEED;
        if (c.flipProgress >= 1.0f)
        {
            c.flipProgress = 1.0f;
            c.flipping = false;
        }
    }
    else
    {
        c.flipProgress -= FLIP_SPEED;
        if (c.flipProgress <= 0.0f)
        {
            c.flipProgress = 0.0f;
            c.flipping = false;
        }
    }
}

void renderHand(
    HandCard hand[],
    float moveArr[],
    float cardXArr[],
    float handY,
    float deckX,
    float deckY,
    unsigned int deckTex,
    int dir        
)
{
    for (int i = 0; i < HAND_SIZE; i++)
    {
        if (!hand[i].occupied) continue;

        // anim getting in the deck
        moveArr[i] = std::min(1.0f, moveArr[i] + MOVE_SPEED * 0.016f);
        float t = easeOutCubic(moveArr[i]);

        float cardX = deckX * (1.f - t) + cardXArr[i] * t;
        float cardY = deckY * (1.f - t) + handY * t;

        // lift anim for selection
        float targetLift = (hand[i].selected ? 0.15f : 0.0f) * dir;
        hand[i].selectLift += (targetLift - hand[i].selectLift) * SELECT_LIFT_SPEED;
        cardY += hand[i].selectLift;

        updateFlip(hand[i]);

        // scale x depends on flipProgress
        float flipScaleX =
            SCALE_X * std::max(0.05f, std::abs(1.f - 2.f * hand[i].flipProgress));

        unsigned int tex =
            (hand[i].flipProgress < 0.5f)
            ? deckTex
            : getCardTextureByID(hand[i].cardID);

        // shadow
        drawQuad(globalShader, tex,
            cardX + 0.01f,
            cardY - 0.01f,
            flipScaleX * 1.01f,
            SCALE_Y * 1.01f,
            globalVAO,
            true);

        // main card
        drawQuad(globalShader, tex,
            cardX,
            cardY,
            flipScaleX,
            SCALE_Y,
            globalVAO,
            false);

        if (hand[i].cornerFold > 0.0f)
        {
            unsigned int faceTex = getCardTextureByID(hand[i].cardID);

            //drawFoldTriangle();  CAN'T DO THIS AAAA
        }
    }
}

static void applyTextureParams(unsigned int tex)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    setTextureParams();
}

void loadAllTextures()
{
    texBG = loadImageToTexture("Resources/table_high_res.jpg");
    texMyInfo = loadImageToTexture("Resources/my_info.png");
    texDeckRed = loadImageToTexture("Resources/back_red_big.png");
    texDeckBlue = loadImageToTexture("Resources/back_blue_big.png");

    texRedWins = loadImageToTexture("Resources/red_wins.png");
    texBlueWins = loadImageToTexture("Resources/blue_wins.png");

    texRespect = loadImageToTexture("Resources/pay_respect_btn.png");
    texRageBtn = loadImageToTexture("Resources/rage_btn.png");

    texRespectPopup = loadImageToTexture("Resources/respect.png");
    texRageAnim[0] = loadImageToTexture("Resources/anger1.png");
    texRageAnim[1] = loadImageToTexture("Resources/anger2.png");
    texRageAnim[2] = loadImageToTexture("Resources/anger3.png");


    applyTextureParams(texBG);
    applyTextureParams(texMyInfo);
    applyTextureParams(texDeckRed);
    applyTextureParams(texDeckBlue);
    applyTextureParams(texRedWins);
    applyTextureParams(texBlueWins);
    applyTextureParams(texRespect);
    applyTextureParams(texRageBtn);
    applyTextureParams(texRespectPopup);

    applyTextureParams(texRageAnim[0]);
    applyTextureParams(texRageAnim[1]);
    applyTextureParams(texRageAnim[2]);

    for (int i = 0; i < 10; i++)
    {
        char path[64];
        sprintf_s(path, "Resources/%d.png", i);

        digitTextures[i] = loadImageToTexture(path);
        applyTextureParams(digitTextures[i]);
    }

    const char* suitNames[4] = { "clubs", "diamond", "hearts", "spades" };

    for (int s = 0; s < 4; s++)
    {
        for (int r = 1; r <= 13; r++)
        {
            char path[128];
            sprintf_s(path, "Resources/%s%d.png", suitNames[s], r);

            cardFaceTextures[s][r] = loadImageToTexture(path);
            applyTextureParams(cardFaceTextures[s][r]);
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

static bool hoverCardInHand(const HandCard& card, float cardX, float cardY, float mx, float my)
{
    return card.occupied && isHover(mx, my, cardX, cardY, SCALE_X, SCALE_Y);
}

// MAIN
int main()
{
    // SETUP
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(
        mode->width, mode->height, "Card game", monitor, nullptr
    );
    if (!window) return endProgram("Prozor nije uspeo da se kreira.");

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
        return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    initFoldVAO();


    // SHADER + QUAD

    unsigned int shader = createShader("texture.vert", "texture.frag");
    globalShader = shader;

    float quad[] = {
        -1.f,  1.f, 0.f, 1.f,
        -1.f, -1.f, 0.f, 0.f,
         1.f, -1.f, 1.f, 0.f,
         1.f,  1.f, 1.f, 1.f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    globalVAO = VAO;

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // CURSOR
    GLFWcursor* cursorDefault = loadImageToCursor("Resources/hand_thin_point.png");
    GLFWcursor* cursorHover = loadImageToCursor("Resources/hand_thin_open.png");
    GLFWcursor* cursorView = loadImageToCursor("Resources/look_d.png");

    glfwSetCursor(window, cursorDefault);

    // GAME START
    loadAllTextures();
    generateDecks();
    initHands();

    // MAIN LOOP
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        // mouse NDC
        double cx, cy;
        int ww, wh;
        glfwGetCursorPos(window, &cx, &cy);
        glfwGetWindowSize(window, &ww, &wh);

        float mx = toNDC_X(cx, ww);
        float my = toNDC_Y(cy, wh);

        // HOVER DETECTION + CURSOR
        GLFWcursor* cur = cursorDefault;

        bool hoverDeck1 = isHover(mx, my, DECK_X, DECK_Y, SCALE_X, SCALE_Y);
        bool hoverDeck2 = isHover(mx, my, DECK_X2, DECK_Y2, SCALE_X, SCALE_Y);

        if (hoverDeck1 || hoverDeck2)
            cur = cursorHover;

        for (int i = 0; i < HAND_SIZE; i++)
        {
            if (hand[i].occupied)
            {
                float t = easeOutCubic(moveToHand[i]);
                float cx = DECK_X * (1 - t) + handCardX[i] * t;
                float cy = DECK_Y * (1 - t) + HAND_Y * t;

                if (hoverCardInHand(hand[i], cx, cy, mx, my))
                    cur = cursorView;
            }
            if (hand2[i].occupied)
            {
                float t = easeOutCubic(moveToHand2[i]);
                float cx = DECK_X2 * (1 - t) + handCardX2[i] * t;
                float cy = DECK_Y2 * (1 - t) + HAND_Y2 * t;

                if (hoverCardInHand(hand2[i], cx, cy, mx, my))
                    cur = cursorView;
            }
        }

        if (roundState != ROUND_ENDGAME)
            glfwSetCursor(window, cur);

        // INPUT
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            resetGame();

        handleLeftClick(mx, my, hoverDeck1, hoverDeck2);
        handleRightClick(mx, my);

        if (!roundLocked)
            updateSelection();

        // UPDATE ANIMATIONS
        updateCornerFold();

        lifted = updateLift(hoverDeck1, lifted);
        lifted2 = updateLift(hoverDeck2, lifted2);

        // ROUND START TRIGGER
        if (selectedIndex1 != -1 && selectedIndex2 != -1 && roundState == ROUND_IDLE)
        {
            startRound();
        }

        // RENDER BG + discard couting
        drawQuad(shader, texBG, 0, 0, 1, 1, VAO);

        // MY INFO
        glUniform1f(glGetUniformLocation(shader, "uAlphaMul"), 0.5f);
        drawQuad(shader, texMyInfo,
            0.90f,  
            0.95f,   
            0.10f,  
            0.10f,   
            VAO,
            false);
        glUniform1f(glGetUniformLocation(shader, "uAlphaMul"), 1.0f);

        if (playedRed.discardTex)  drawQuad(shader, playedRed.discardTex, playedRed.discardX, playedRed.discardY, SCALE_X, SCALE_Y, VAO);
        if (playedBlue.discardTex) drawQuad(shader, playedBlue.discardTex, playedBlue.discardX, playedBlue.discardY, SCALE_X, SCALE_Y, VAO);

        if (!discardRed.empty())  drawNumber(shader, discardRed.size(), playedRed.discardX - 0.02f, playedRed.discardY + 0.35f, 0.07f, VAO);
        if (!discardBlue.empty()) drawNumber(shader, discardBlue.size(), playedBlue.discardX - 0.02f, playedBlue.discardY - 0.35f, 0.07f, VAO);

        // RENDER DECKS AND HANDS
        renderDeckHover(texDeckRed, DECK_X, DECK_Y, lifted);
        renderDeckHover(texDeckBlue, DECK_X2, DECK_Y2, lifted2);

        renderHand(hand, moveToHand, handCardX, HAND_Y, DECK_X, DECK_Y, texDeckRed, +1);
        renderHand(hand2, moveToHand2, handCardX2, HAND_Y2, DECK_X2, DECK_Y2, texDeckBlue, -1);

        // ROUND STATE CHANGER
        switch (roundState)
        {
        case ROUND_REVEALING:
            updateRoundReveal(mx, my);
            break;

        case ROUND_WAIT:
            updateRoundWait(mx, my);
            break;

        case ROUND_COLLECTING:
            updateRoundCollect(mx, my);
            break;

        case ROUND_ENDGAME:
            updateRoundEndgame(mx, my, ww, wh);
            break;

        default:
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        limitFPS();
    }

    glfwTerminate();
    return 0;
}