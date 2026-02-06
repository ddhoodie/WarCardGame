#include "GameState.h"
#include <algorithm>
#include <string>

// ===== globals (extern definicije) =====
bool gDepthEnabled = true;
bool gCullEnabled = true;
bool gLightEnabled = true;

Transform gTableTr{};
Transform gDeckRedTr{};
Transform gDeckBlueTr{};

CardEntity hand3D[HAND_SIZE]{};
CardEntity hand3D_2[HAND_SIZE]{};

unsigned int texMyInfo = 0;
unsigned int texTable = 0;
unsigned int texDeckRed = 0;
unsigned int texDeckBlue = 0;

unsigned int cardFaceTextures[4][14] = { 0 };

// ===== blackjack / deck logic =====

std::string cardPath(const CardId& c)
{
    const char* suitName[4] = { "clubs", "diamond", "hearts", "spades" };
    return std::string("Resources/") + suitName[c.suit] + std::to_string(c.rank) + ".png";
}

void Deck52::resetAndShuffle(std::mt19937& rng)
{
    order.clear();
    order.reserve(52);

    for (int s = 0; s < 4; ++s)
        for (int r = 1; r <= 13; ++r)
            order.push_back({ s, r });

    std::shuffle(order.begin(), order.end(), rng);
    top = 0;
}

bool Deck52::empty() const
{
    return top >= (int)order.size();
}

CardId Deck52::draw()
{
    return order[top++]; // pretpostavka: pozivac proverio empty()
}

static int cardValueNoAceSpecial(int rank)
{
    if (rank == 1) return 1;
    if (rank >= 10) return 10;
    return rank;
}

int bestScoreAce1or10(const std::vector<int>& handIdx, const std::vector<int>& cardRanks)
{
    int sum = 0;
    int aces = 0;

    for (int idx : handIdx) {
        int r = cardRanks[idx];
        if (r == 1) aces++;
        sum += cardValueNoAceSpecial(r);
    }

    int best = sum;
    for (int k = 1; k <= aces; ++k) {
        int cand = sum + 9 * k;      // ace 10 umesto 1 => +9
        if (cand <= 21) best = std::max(best, cand);
    }
    return best;
}