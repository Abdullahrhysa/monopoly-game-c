#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"

int main() {
    srand(time(NULL));

    struct Player players[NUM_PLAYERS];
    initPlayers(players);

    printf("MONOPOLY-LK Simulation\n");
    for (int i = 0; i < NUM_PLAYERS; i++) {
        printf("Player %d : %s\n", i + 1, players[i].name);
    }
    printf("Each player begins with LKR %d.\n\n", STARTING_CASH);

    int order[NUM_PLAYERS];
    determineTurnOrder(players, order);

    struct Property board[NUM_SQUARES];
    initBoard(board);

    playGame(players, board, order);

    return 0;
}