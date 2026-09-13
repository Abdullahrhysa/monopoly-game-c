#include <stdio.h>
#include <string.h>
#include "types.h"


char* strategyName(enum Strategy s) {
    if (s == AGGRESSIVE) return "Aggressive Investor";
    if (s == CONSERVATIVE) return "Conservative Banker";
    if (s == RISK_TAKER) return "Risk Taker";
    return "Opportunistic Trader";   
}


void initPlayers(struct Player players[]) {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].cash = STARTING_CASH;
        players[i].position = 0;         
        players[i].hasLoan = 0;
        players[i].loanAmount = 0;
        players[i].loanInterestRate = 0;
        players[i].loanRoundsLeft = 0;
        players[i].inJail = 0;
        players[i].jailTurnsLeft = 0;
        players[i].isBankrupt = 0;
        players[i].eventCard = -1;
        players[i].eventCardEndRound = 0;
        for (int j = 0; j < NUM_SQUARES; j++) {
            players[i].ownedSquares[j] = 0;  
        }
    }
    players[0].strategy = AGGRESSIVE;
    players[1].strategy = CONSERVATIVE;
    players[2].strategy = RISK_TAKER;
    players[3].strategy = OPPORTUNISTIC;

    strcpy(players[0].name, strategyName(AGGRESSIVE));
    strcpy(players[1].name, strategyName(CONSERVATIVE));
    strcpy(players[2].name, strategyName(RISK_TAKER));
    strcpy(players[3].name, strategyName(OPPORTUNISTIC));
}

// Estimates rent for buy-decision purposes only 
int estimateRent(struct Property *square) {
    if (square->type == PROPERTY) return square->baseRent;
    if (square->type == RAILWAY) return 250;    
    if (square->type == UTILITY) return 28;      // 4 x average dice roll (~7)
    return 0;
}

int shouldBuy(struct Player *player, int playerIndex, struct Property board[], struct Property *square) {
    if (antiSpeculationActive && square->type == PROPERTY &&
        countUndevelopedProperties(board, playerIndex) >= 3) {
        return 0;
    }
    int rent = estimateRent(square);
    switch (player->strategy) {
        case AGGRESSIVE:
            return (player->cash - square->price) >= rent;
        case CONSERVATIVE:
            return square->price <= player->cash / 2;
        case RISK_TAKER:
            return player->cash >= square->price;
        case OPPORTUNISTIC:
            return (player->cash >= square->price) && (rent * 10 >= square->price / 2);
        default:
            return 0;
    }
}
void buyProperty(struct Player *player, int playerIndex, struct Property *square) {
    player->cash -= square->price;
    square->owner = playerIndex;
    player->ownedSquares[player->position] = 1;
    printf("%s purchased %s for LKR %d.\n", player->name, square->name, square->price);
    printf("Remaining Balance : LKR %d.\n", player->cash);
}

int getGroupIndex(int squareIndex) {
    int colorGroups[8][3] = {
        {1, 3, -1},      // Brown (only 2 properties, -1 = unused slot)
        {6, 8, 9},        // Light Blue
        {11, 13, 14},     // Pink
        {16, 18, 19},     // Orange
        {21, 23, 24},     // Red
        {26, 27, 29},     // Yellow
        {31, 32, 34},     // Green
        {37, 39, -1}      // Dark Blue
    };
    int groupSizes[8] = {2, 3, 3, 3, 3, 3, 3, 2};

    for (int g = 0; g < 8; g++) {
        for (int j = 0; j < groupSizes[g]; j++) {
            if (colorGroups[g][j] == squareIndex) return g;
        }
    }
    return -1;   // not a grouped property (railway/utility/etc.)
}

int hasMonopoly(struct Property board[], int ownerIndex, int groupIndex) {
    int colorGroups[8][3] = {
        {1, 3, -1}, {6, 8, 9}, {11, 13, 14}, {16, 18, 19},
        {21, 23, 24}, {26, 27, 29}, {31, 32, 34}, {37, 39, -1}
    };
    int groupSizes[8] = {2, 3, 3, 3, 3, 3, 3, 2};

    for (int j = 0; j < groupSizes[groupIndex]; j++) {
        int idx = colorGroups[groupIndex][j];
        if (board[idx].owner != ownerIndex) return 0;   // Rule 8: needs EVERY property
    }
    return 1;
}

int countUndevelopedProperties(struct Property board[], int playerIndex) {
    int count = 0;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].type == PROPERTY && board[i].owner == playerIndex &&
            board[i].houses == 0 && !board[i].hasHotel) {
            count++;
        }
    }
    return count;
}

void buildOnGroup(struct Player *player, int playerIndex, struct Property board[], int groupIndex) {
    int colorGroups[8][3] = {
        {1, 3, -1}, {6, 8, 9}, {11, 13, 14}, {16, 18, 19},
        {21, 23, 24}, {26, 27, 29}, {31, 32, 34}, {37, 39, -1}
    };
    int groupSizes[8] = {2, 3, 3, 3, 3, 3, 3, 2};

    // How far this strategy wants to build: 4 = max houses only, 5 = hotel
    int targetLevel;
    switch (player->strategy) {
        case AGGRESSIVE:    targetLevel = 5; break;  // converts to hotels ASAP
        case RISK_TAKER:    targetLevel = 5; break;  //  hotels as early as possible
        case CONSERVATIVE:  targetLevel = player->hasLoan ? 4 : 5; break;  //  no hotels until loans settled
        case OPPORTUNISTIC: targetLevel = 4; break;  //  balanced portfolio, houses only
        default: targetLevel = 0;
    }

    int builtSomething = 1;
    while (builtSomething) {
        builtSomething = 0;

        // find the LEAST developed property in the group still below target - builds evenly
        int minLevel = 100, minIdx = -1;
        for (int j = 0; j < groupSizes[groupIndex]; j++) {
            int idx = colorGroups[groupIndex][j];
            int level = board[idx].hasHotel ? 5 : board[idx].houses;
            if (level < targetLevel && level < minLevel) {
                minLevel = level;
                minIdx = idx;
            }
        }
        if (minIdx == -1) break;   // everyone in group already at target level

        struct Property *p = &board[minIdx];
       if (minLevel < 4 && player->cash >= p->houseCost * inflationMultiplier / 100) {
            int cost = p->houseCost * inflationMultiplier / 100;
            player->cash -= cost;
            p->houses++;
            printf("%s constructed one house on %s.\n", player->name, p->name);
            printf("Construction Cost : LKR %d.\n", cost);
            builtSomething = 1;
        } else if (minLevel == 4 && player->cash >= p->hotelCost * inflationMultiplier / 100) {
            int cost = p->hotelCost * inflationMultiplier / 100;
            player->cash -= cost;
            p->houses = 0;
            p->hasHotel = 1;
            printf("%s upgraded %s to a Hotel.\n", player->name, p->name);
            builtSomething = 1;
        }
    }
}

void constructBuildings(struct Player *player, int playerIndex, struct Property board[]) {
    for (int g = 0; g < 8; g++) {
        if (hasMonopoly(board, playerIndex, g)) {
            buildOnGroup(player, playerIndex, board, g);
        }
    }
}