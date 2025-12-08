#include "../Header/GameState.h"
#include "../Header/Util.h"
#include "../Header/Helper.h"

void startRound()
{
    roundLocked = true;


    // PLAYER RED 
    {
        HandCard& h = hand[selectedIndex1];

        playedRed.tex = getCardTextureByID(h.cardID);
        playedRed.startX = handCardX[selectedIndex1];
        playedRed.startY = HAND_Y;

        playedRed.progress = 0.0f;
        playedRed.flipProgress = h.isFlipped ? 1.0f : 0.0f;

        playedRed.rank = (h.cardID % 13) + 1;
        playedRed.suit = h.cardID / 13;

        hasPlayedRed = true;

        h.occupied = false;
        h.selected = false;
        selectedIndex1 = -1;
        cardsInHand--;
    }

    // PLAYER BLUE
    {
        HandCard& h = hand2[selectedIndex2];

        playedBlue.tex = getCardTextureByID(h.cardID);
        playedBlue.startX = handCardX2[selectedIndex2];
        playedBlue.startY = HAND_Y2;

        playedBlue.progress = 0.0f;
        playedBlue.flipProgress = h.isFlipped ? 1.0f : 0.0f;

        playedBlue.rank = (h.cardID % 13) + 1;
        playedBlue.suit = h.cardID / 13;

        hasPlayedBlue = true;

        h.occupied = false;
        h.selected = false;
        selectedIndex2 = -1;
        cardsInHand2--;
    }

    resolveTimer = 0.f;
    collectTimer = 0.f;
    roundState = ROUND_REVEALING;
}

static void updateRevealSingle(PlayedCard& pc, unsigned int deckTex, float targetX, float targetY)
{
    if (pc.flipProgress < 1.0f)
    {
        pc.flipProgress += FLIP_SPEED;
        if (pc.flipProgress > 1.0f)
            pc.flipProgress = 1.0f;
    }
    else if (pc.progress < 1.0f)
    {
        pc.progress += PLAY_SPEED;
        if (pc.progress > 1.0f)
            pc.progress = 1.0f;
    }

    float t = easeOutCubic(pc.progress);

    float x = pc.startX * (1.f - t) + targetX * t;
    float y = pc.startY * (1.f - t) + targetY * t;

    if (pc.progress >= 1.f)
    {
        pc.tableX = targetX;
        pc.tableY = targetY;
    }

    float flipScaleX =
        SCALE_X * std::max(0.05f, std::abs(1.f - 2.f * pc.flipProgress));

    unsigned int tex =
        (pc.flipProgress < 0.5f ? deckTex : pc.tex);

    drawQuad(globalShader, tex, x, y, flipScaleX, SCALE_Y, globalVAO);
}

void updateRoundReveal(float mx, float my)
{
    bool doneRed = true;
    bool doneBlue = true;

    if (hasPlayedRed)
    {
        updateRevealSingle(playedRed, texDeckRed, 0.15f, -0.15f);
        doneRed = (playedRed.progress >= 1.f && playedRed.flipProgress >= 1.f);
    }

    if (hasPlayedBlue)
    {
        updateRevealSingle(playedBlue, texDeckBlue, -0.15f, 0.15f);
        doneBlue = (playedBlue.progress >= 1.f && playedBlue.flipProgress >= 1.f);
    }

    if (doneRed && doneBlue)
    {
        resolveTimer = 0.f;
        roundState = ROUND_WAIT;
    }
}

void updateRoundWait(float mx, float my)
{
    resolveTimer += 0.016f;

    if (hasPlayedRed)
        drawQuad(globalShader, playedRed.tex,
            playedRed.tableX, playedRed.tableY,
            SCALE_X, SCALE_Y, globalVAO);

    if (hasPlayedBlue)
        drawQuad(globalShader, playedBlue.tex,
            playedBlue.tableX, playedBlue.tableY,
            SCALE_X, SCALE_Y, globalVAO);

    if (resolveTimer >= 1.f)
    {
        roundState = ROUND_COLLECTING;
        collectTimer = 0.f;
        collectPhase = 0;
    }
}

void updateRoundCollect(float mx, float my)
{
    bool redWins = playedRed.rank > playedBlue.rank;

    float loseX = redWins ? playedBlue.tableX : playedRed.tableX;
    float loseY = redWins ? playedBlue.tableY : playedRed.tableY;

    float winX = redWins ? playedRed.tableX : playedBlue.tableX;
    float winY = redWins ? playedRed.tableY : playedBlue.tableY;

    float discardX = redWins ? 0.85f : -0.85f;
    float discardY = redWins ? -0.10f : 0.10f;


    // PHASE 0 - winner cardgoing over loser
    if (collectPhase == 0)
    {
        collectTimer += COLLECT_SPEED;
        float t = easeOutCubic(collectTimer);

        float curWinX = lerp(winX, loseX, t);
        float curWinY = lerp(winY, loseY, t);

        drawQuad(globalShader,
            redWins ? playedBlue.tex : playedRed.tex,
            loseX, loseY, SCALE_X, SCALE_Y, globalVAO);

        drawQuad(globalShader,
            redWins ? playedRed.tex : playedBlue.tex,
            curWinX, curWinY, SCALE_X, SCALE_Y, globalVAO);

        if (t >= 1.f)
        {
            collectPhase = 1;
            collectTimer = 0.f;
        }
        return;
    }

    // PHASE 1 – in discard pile both to player who won
    if (collectPhase == 1)
    {
        collectTimer += COLLECT_SPEED * 0.75f;
        float t = easeOutCubic(collectTimer);

        float x = lerp(loseX, discardX, t);
        float y = lerp(loseY, discardY, t);

        unsigned int winnerTex =
            redWins ? playedRed.tex : playedBlue.tex;

        drawQuad(globalShader, winnerTex, x, y, SCALE_X, SCALE_Y, globalVAO);

        if (t >= 1.f)
        {
            if (redWins)
            {
                playedRed.discardX = discardX;
                playedRed.discardY = discardY;
                playedRed.discardTex = winnerTex;

                discardRed.push_back({ playedRed.suit, playedRed.rank });
                discardRed.push_back({ playedBlue.suit, playedBlue.rank });
            }
            else
            {
                playedBlue.discardX = discardX;
                playedBlue.discardY = discardY;
                playedBlue.discardTex = winnerTex;

                discardBlue.push_back({ playedRed.suit, playedRed.rank });
                discardBlue.push_back({ playedBlue.suit, playedBlue.rank });
            }

            hasPlayedRed = false;
            hasPlayedBlue = false;
            playedRed.tex = 0;
            playedBlue.tex = 0;

            bool noRed = (cardsInHand == 0 && deck.empty());
            bool noBlue = (cardsInHand2 == 0 && deck2.empty());

            if (noRed && noBlue)
            {
                gameOver = true;
                redWinner = (discardRed.size() > discardBlue.size());
                roundState = ROUND_ENDGAME;
                return;
            }

            collectPhase = 0;
            roundLocked = false;
            roundState = ROUND_IDLE;
        }

        return;
    }
}

void updateRoundEndgame(float mx, float my, int ww, int wh)
{
    unsigned int winTex = redWinner ? texRedWins : texBlueWins;

    drawQuad(globalShader, winTex,
        0.0f, 0.2f,
        0.5f, 0.3f,
        globalVAO);

    float btnScaleX = 0.1f;
    float btnScaleY = 0.1f;

    float leftBtnX = -0.30f;
    float rightBtnX = 0.30f;
    float btnY = -0.25f;

    drawQuad(globalShader, texRespect, leftBtnX, btnY, btnScaleX, btnScaleY, globalVAO);
    drawQuad(globalShader, texRageBtn, rightBtnX, btnY, btnScaleX, btnScaleY, globalVAO);

    bool hoverRespect = isHover(mx, my, leftBtnX, btnY, btnScaleX, btnScaleY);
    bool hoverRage = isHover(mx, my, rightBtnX, btnY, btnScaleX, btnScaleY);

    if (hoverRespect || hoverRage)
        glfwSetCursor(glfwGetCurrentContext(),
            loadImageToCursor("Resources/hand_thin_open.png"));

    if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        double rx, ry;
        glfwGetCursorPos(glfwGetCurrentContext(), &rx, &ry);

        float mx2 = (rx / ww) * 2.f - 1.f;
        float my2 = -((ry / wh) * 2.f - 1.f);

        if (isHover(mx2, my2, leftBtnX, btnY, btnScaleX, btnScaleY))
            startRespectAnim();

        if (isHover(mx2, my2, rightBtnX, btnY, btnScaleX, btnScaleY))
            startRageAnim(redWinner);
    }

    updateFloatingAnim(respectAnim, RESPECT_ANIM_SPEED);
    updateFloatingAnim(rageAnim, RAGE_ANIM_SPEED);
}
