#include <stdio.h>
#include <stdlib.h>
#include "types.h"

int rollDice() {
    return (rand() % 6 + 1) + (rand() % 6 + 1);   // two six-sided dice togetherr
}

void determineTurnOrder(struct Player players[], int order[]) {
    int rolls[NUM_PLAYERS];

    for (int i = 0; i < NUM_PLAYERS; i++) {
        rolls[i] = rollDice();
        printf("%s rolls %d.\n", players[i].name, rolls[i]);
    }

    int maxRoll = -1;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (rolls[i] > maxRoll) maxRoll = rolls[i];
    }

    int tieCount = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (rolls[i] == maxRoll) tieCount++;
    }

    // tied players reroll, repeatedly until the tie breaks - only for the first turn
    while (tieCount > 1) {
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (rolls[i] == maxRoll) {
                rolls[i] = rollDice();
                printf("%s rerolls %d.\n", players[i].name, rolls[i]);
            }
        }
        maxRoll = -1;
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (rolls[i] > maxRoll) maxRoll = rolls[i];
        }
        tieCount = 0;
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (rolls[i] == maxRoll) tieCount++;
        }
    }

    int winner = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (rolls[i] == maxRoll) { winner = i; break; }
    }

    printf("%s will begin the game.\n", players[winner].name);
    printf("Turn order:\n");
    for (int i = 0; i < NUM_PLAYERS; i++) {
        int idx = (winner + i) % NUM_PLAYERS;   // wrap around, starting at winner
        order[i] = idx;
        printf("%s\n", players[idx].name);
    }
}

// Shared game state - deliberately global since multiple independent functions

int activeMarketBoomGroup = -1;
int activeMarketBoomEndRound = -1;
int activeMarketDeclineGroup = -1;
int activeMarketDeclineEndRound = -1;
int activeRegionalCard = -1;
int activeRegionalCardEndRound = -1;
int currentInflationRate = 0;   // whole-number percent
int railwayRentMultiplier = 100;
int utilityRentMultiplier = 100;
int currentIncomeTaxRate = 15;
int inflationMultiplier = 100;   // cumulative percent, 100 = baseline, compounds via Rule-LK 14
int antiSpeculationActive = 0;
int insurancePremiumMultiplier = 100;


void movePlayer(struct Player *player, int diceRoll) {
    int oldPosition = player->position;
    int newPosition = (oldPosition + diceRoll) % NUM_SQUARES;
    player->position = newPosition;

    printf("%s moves from Square %d to Square %d.\n", player->name, oldPosition, newPosition);

    // the player passed or landed on GO
    if (newPosition < oldPosition) {
        player->cash += 2000;
        printf("%s passed GO.\n", player->name);
        printf("Collected LKR 2000.\n");
        printf("Current Balance : LKR %d.\n", player->cash);
    }
}

void sendToJail(struct Player *player) {
    player->position = 10;
    player->inJail = 1;
    player->jailTurnsLeft = 3;
    printf("%s has been sent to Jail.\n", player->name);
}

void handleLanding(struct Player *player, int playerIndex, struct Player players[], struct Property board[], int diceRoll, int round) {
    struct Property *square = &board[player->position];

    switch (square->type) {
        case SPECIAL:
            if (player->position == 30) {
                sendToJail(player);
            }
            break;
        case PROPERTY:
        case RAILWAY:
        case UTILITY:
            if (square->owner == -1) {
                printf("%s landed on %s.\n", player->name, square->name);
               if (shouldBuy(player, playerIndex, board, square)) {
                    buyProperty(player, playerIndex, square);
                } else {
                printf("%s declined to purchase %s.\n", player->name, square->name);
                runAuction(players, board, square);
            }
           } else if (square->owner != playerIndex) {
                payRent(players, playerIndex, board, square, diceRoll);
            } else {
                printf("%s landed on %s (already owned).\n", player->name, square->name);
                if (square->type == PROPERTY && square->age > 50) {
                    int depreciationSteps = (square->age - 50) / 5;
                    int threshold = (player->strategy == CONSERVATIVE) ? 10 :
                                     (player->strategy == OPPORTUNISTIC) ? 15 : 20;
                    if (depreciationSteps >= threshold && player->cash >= square->price / 10) {
                        renovateProperty(player, square);
                    }
                }
            }
            break;
        case TAX:
            payIncomeTax(player, playerIndex, players, board);
            break;
        case EVENT:
            if (player->position == 2) {
                payCommunityDevelopmentFund(player, playerIndex, players, board);
            } else {
                drawEventCard(player, playerIndex, players, board, round);
            }
            break;
        case BANK:
    handleBankVisit(player, playerIndex, board);
    break;
        case INSURANCE:
            handleInsuranceVisit(player, playerIndex, board);
            break;
    }
}
void playTurn(struct Player *player, int playerIndex, struct Player players[], struct Property board[], int round) {
    if (player->isBankrupt) return;
    attemptAutoRepair(player, playerIndex, players, board);
    maintainBuildings(player, playerIndex, board);

    if (player->inJail) {
        int roll1 = rand() % 6 + 1;
        int roll2 = rand() % 6 + 1;
        printf("%s is in Jail.\n", player->name);
        if (roll1 == roll2) {
            player->inJail = 0;
            printf("%s rolled doubles (%d, %d) and is released.\n", player->name, roll1, roll2);
            movePlayer(player, roll1 + roll2);
            handleLanding(player, playerIndex, players, board, roll1 + roll2, round);
            checkBankruptcy(player, playerIndex, players, board);
        } else {
            player->jailTurnsLeft--;
            printf("%s remains in Jail. Turns left : %d.\n", player->name, player->jailTurnsLeft);
            if (player->jailTurnsLeft <= 0) {
                player->inJail = 0;
                player->cash -= 300;
                printf("%s served time and paid bail of LKR 300.\n", player->name);
                checkBankruptcy(player, playerIndex, players, board);
            }
        }
        printf("\n");
        return;
    }

    int diceRoll = rollDice();
    printf("%s rolled %d.\n", player->name, diceRoll);

    movePlayer(player, diceRoll);
    handleLanding(player, playerIndex, players, board, diceRoll, round);
    checkBankruptcy(player, playerIndex, players, board);

    if (!player->isBankrupt) {
        constructBuildings(player, playerIndex, board);
    }

    printf("\n");
}

void printRoundSummary(int round, struct Player players[], struct Property board[]) {
    printf("=============================================\n");
    printf("Round %d Summary\n", round);
    printf("=============================================\n");
    for (int i = 0; i < NUM_PLAYERS; i++) {
        printf("%s\n", players[i].name);
        printf("Cash : LKR %d\n", players[i].cash);
        printf("Net Worth : LKR %d\n", calculateNetWorth(&players[i], i, board));
        printf("Properties : %d\n", countPlayerProperties(&players[i]));
        printf("Hotels : %d\n", countPlayerHotels(board, i));
        if (players[i].hasLoan) {
            printf("Outstanding Loan : LKR %d\n", players[i].loanAmount);
        } else {
            printf("Outstanding Loan : None\n");
        }
        if (i < NUM_PLAYERS - 1) printf("---------------------------------------------\n");
    }
    printf("=============================================\n\n");
}

int countSolventPlayers(struct Player players[]) {
    int count = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (!players[i].isBankrupt) count++;
    }
    return count;
}

void playGame(struct Player players[], struct Property board[], int order[]) {
    int round;
    for (round = 1; round <= MAX_ROUNDS; round++) {
        for (int t = 0; t < NUM_PLAYERS; t++) {
            int idx = order[t];
            playTurn(&players[idx], idx, players, board, round);
        }
        checkEventCardExpiry(players, round);
        decayBuildingCondition(board);
        updateInsurancePolicies(board, players);
        processLoans(players, board);
        processLoans(players, board);

        if (round % 10 == 0) {
            triggerDisaster(players, board);
            applyPropertyDepreciation(board, round);
            reviewPropertyMarket(board, round);
            applyInflation(round);
        }
        if (round % 15 == 0) {
            triggerEconomicEvent(players, board);
            applyRegionalDevelopmentCard(board, round);
        }
        if (round % 20 == 0) {
            triggerGovernmentRegulation(players, board);
        }

        printRoundSummary(round, players, board);
        printMarketConditions(round);
        if (countSolventPlayers(players) <= 1) break;
    }

    printf("=============================================\n");
    printf("GAME OVER\n");

    int winner = -1, bestNetWorth = -1000000000;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (!players[i].isBankrupt) {
            int nw = calculateNetWorth(&players[i], i, board);
            if (nw > bestNetWorth) {
                bestNetWorth = nw;
                winner = i;
            }
        }
    }

    printf("Winner\n%s\n", players[winner].name);
    printf("Total Cash\nLKR %d\n", players[winner].cash);
    printf("Net Worth\nLKR %d\n", bestNetWorth);
    printf("=============================================\n");
}

void applyPropertyDepreciation(struct Property board[], int round) {
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].type == PROPERTY && board[i].owner != -1) {
            board[i].age += 10;   // this function runs every 10 rounds, so age advances by 10 each call

            if (board[i].age > 50) {
                int depreciationSteps = (board[i].age - 50) / 5;
                int depreciationPercent = depreciationSteps * 1;
                if (depreciationPercent > 30) depreciationPercent = 30;   

                int currentValue = board[i].price * (100 - depreciationPercent) / 100;
                printf("Property\n%s\nhas depreciated by %d%%.\n", board[i].name, depreciationPercent);
                printf("Current Value\nLKR %d.\n", currentValue);
            }
        }
    }
}

void applyInflation(int round) {
    int possibleRates[6] = {-3, 0, 2, 5, 8, 12};
    int rateIndex = rand() % 6;
    currentInflationRate = possibleRates[rateIndex]; 
    // compounding - New Value = Previous Value x (1 + rate)
    inflationMultiplier = inflationMultiplier * (100 + currentInflationRate) / 100;

    printf("Inflation Rate Update : %d%%\n", currentInflationRate);
    printf("Cumulative Inflation Index : %d%%\n", inflationMultiplier);
}

void reviewPropertyMarket(struct Property board[], int round) {
    if (activeMarketBoomGroup != -1 && round >= activeMarketBoomEndRound) {
        activeMarketBoomGroup = -1;   // boom expires after 10 rounds
    }
    if (activeMarketDeclineGroup != -1 && round >= activeMarketDeclineEndRound) {
        activeMarketDeclineGroup = -1;   // decline expires after 10 rounds
    }

    if (activeMarketBoomGroup == -1) {
        activeMarketBoomGroup = rand() % 8;
        activeMarketBoomEndRound = round + 10;
    }
    if (activeMarketDeclineGroup == -1) {
        int newDecline = rand() % 8;
        while (newDecline == activeMarketBoomGroup) newDecline = rand() % 8;   // can't be the same group
        activeMarketDeclineGroup = newDecline;
        activeMarketDeclineEndRound = round + 10;
    }

    char *groupNames[8] = {"Brown", "Light Blue", "Pink", "Orange", "Red", "Yellow", "Green", "Dark Blue"};
    printf("Market Boom\n-------------\n%s (+25%% rent)\nRounds Remaining : %d\n", groupNames[activeMarketBoomGroup], activeMarketBoomEndRound - round);
    printf("Market Decline\n----------------\n%s (-20%% rent)\nRounds Remaining : %d\n", groupNames[activeMarketDeclineGroup], activeMarketDeclineEndRound - round);
}


void decayBuildingCondition(struct Property board[]) {
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].type == PROPERTY && (board[i].houses > 0 || board[i].hasHotel)) {
            if (board[i].buildingCondition > 0) board[i].buildingCondition -= 2;   // Rule-LK 25
            if (board[i].buildingCondition < 0) board[i].buildingCondition = 0;
            board[i].roundsNeglected++;

            if (board[i].roundsNeglected > 20 && !board[i].structuralDamage) {   // Rule-LK 28
                board[i].structuralDamage = 1;
                board[i].price = board[i].price * 85 / 100;
                printf("%s has suffered structural damage from neglect.\n", board[i].name);
            }
        }
    }
}

void updateInsurancePolicies(struct Property board[], struct Player players[]) {
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].insuranceType != 0) {
            board[i].insuranceRoundsLeft--;

            if (board[i].insuranceRoundsLeft == 3) {
                printf("Insurance policy on %s expires in 3 rounds.\n", board[i].name);
            }

            if (board[i].insuranceRoundsLeft <= 0) {
                printf("Insurance policy on %s has expired.\n", board[i].name);
                board[i].insuranceType = 0;
            }
        }
    }
}

void checkEventCardExpiry(struct Player players[], int round) {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (players[i].eventCard != -1 && round >= players[i].eventCardEndRound) {
            printf("%s's National Event Card effect has expired.\n", players[i].name);
            players[i].eventCard = -1;
        }
    }
}