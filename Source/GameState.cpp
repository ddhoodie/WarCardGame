#include "../Header/GameState.h"
#include "../Header/Helper.h"
#include "../Header/Util.h"

#include <GLFW/glfw3.h>
#include <algorithm>

// ----------------- DEFINICIJE GLOBALA -----------------

unsigned int globalShader = 0;
unsigned int globalVAO = 0;

// teksture
unsigned int texMyInfo = 0;
unsigned int texBG = 0;
unsigned int texDeckRed = 0;
unsigned int texDeckBlue = 0;
unsigned int texRedWins = 0;
unsigned int texBlueWins = 0;
unsigned int texRespect = 0;
unsigned int texRageBtn = 0;

unsigned int digitTextures[10] = { 0 };
unsigned int cardFaceTextures[4][14] = { 0 };

unsigned int texRespectPopup = 0;
unsigned int texRageAnim[3] = { 0, 0, 0 };

// animacija špila
float lifted = 0.0f;
float lifted2 = 0.0f;

// ruke
float handCardX[HAND_SIZE];
float moveToHand[HAND_SIZE] = { 0.0f };
int   cardsInHand = 0;

float handCardX2[HAND_SIZE];
float moveToHand2[HAND_SIZE] = { 0.0f };
int   cardsInHand2 = 0;

HandCard hand[HAND_SIZE];
HandCard hand2[HAND_SIZE];

std::vector<Card> deck;
std::vector<Card> deck2;

// odigrane karte
PlayedCard playedRed;
PlayedCard playedBlue;
bool hasPlayedRed = false;
bool hasPlayedBlue = false;

// izbor karata
int  selectedIndex1 = -1;
int  selectedIndex2 = -1;
bool roundLocked = false;
float resolveTimer = 0.0f;

// discard
std::vector<Card> discardRed;
std::vector<Card> discardBlue;

// stanje runde / igre
RoundState roundState = ROUND_IDLE;
float collectTimer = 0.0f;
int   collectPhase = 0;
float collectFlip = 0.0f;

bool gameOver = false;
bool redWinner = false;

// floating animacije
FloatingAnim respectAnim;
FloatingAnim rageAnim;

// FPS
double       lastTimeForRefresh = 0.0;
const double FPS = 75.0;

// ----------------- FUNKCIJE -----------------

void limitFPS()
{
    while (glfwGetTime() < lastTimeForRefresh + 1.0 / FPS) {}
    lastTimeForRefresh += 1.0 / FPS;
}

void generateDecks()
{
    deck.clear();
    deck2.clear();

    deck.reserve(52);
    deck2.reserve(52);

    for (int s = 0; s < 4; s++) {
        for (int r = 1; r <= 13; r++) {
            deck.push_back({ s, r });
            deck2.push_back({ s, r });
        }
    }

    std::random_shuffle(deck.begin(), deck.end());
    std::random_shuffle(deck2.begin(), deck2.end());
}

void startRespectAnim()
{
    respectAnim.tex = texRespectPopup;
    respectAnim.x = 0.0f;
    respectAnim.y = -0.1f;
    respectAnim.targetX = 0.0f;
    respectAnim.targetY = 0.35f;
    respectAnim.scale = 0.25f;
    respectAnim.progress = 0.0f;
    respectAnim.active = true;
}

void startRageAnim(bool redWinnerParam)
{
    int idx = rand() % 3;
    rageAnim.tex = texRageAnim[idx];

    if (redWinnerParam) {
        rageAnim.x = (rand() % 200) / 100.0f - 1.0f;
        rageAnim.y = 1.2f;      // iznad ekrana
    }
    else {
        rageAnim.x = (rand() % 200) / 100.0f - 1.0f;
        rageAnim.y = -1.2f;     // ispod ekrana
    }

    rageAnim.targetX = 0.0f;
    rageAnim.targetY = 0.25f;
    rageAnim.scale = 0.3f;
    rageAnim.progress = 0.0f;
    rageAnim.active = true;
}

void updateFloatingAnim(FloatingAnim& a, float speed)
{
    if (!a.active) return;

    a.progress += speed;
    if (a.progress >= 1.0f) {
        a.active = false;
        return;
    }

    float t = easeOutCubic(a.progress);
    float curX = lerp(a.x, a.targetX, t);
    float curY = lerp(a.y, a.targetY, t);

    drawQuad(globalShader, a.tex, curX, curY, a.scale, a.scale, globalVAO);
}

void initHands()
{
    // reset ruku
    for (int i = 0; i < HAND_SIZE; i++)
    {
        hand[i] = HandCard();
        hand2[i] = HandCard();

        moveToHand[i] = 0.0f;
        moveToHand2[i] = 0.0f;
    }

    cardsInHand = 0;
    cardsInHand2 = 0;

    // pozicije slotova u ruci (isti kod kao u starom main-u)
    for (int i = 0; i < HAND_SIZE; i++)
        handCardX[i] = -1.0f + 0.15f + i * 0.28f;

    float mirrorStart = 1.0f - 0.15f - (HAND_SIZE - 1) * 0.28f;
    for (int i = 0; i < HAND_SIZE; i++)
        handCardX2[i] = mirrorStart + i * 0.28f;
}

void resetGame()
{
    initHands();

    playedRed = PlayedCard();
    playedBlue = PlayedCard();
    hasPlayedRed = false;
    hasPlayedBlue = false;

    discardRed.clear();
    discardBlue.clear();

    selectedIndex1 = -1;
    selectedIndex2 = -1;
    roundLocked = false;

    resolveTimer = 0.0f;
    collectTimer = 0.0f;
    collectPhase = 0;
    collectFlip = 0.0f;

    gameOver = false;
    redWinner = false;

    roundState = ROUND_IDLE;

    generateDecks();
}
