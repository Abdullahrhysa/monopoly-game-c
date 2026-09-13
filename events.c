#include <stdio.h>
#include <stdlib.h>
#include "types.h"

void triggerGovernmentRegulation(struct Player players[], struct Property board[]) {
    char *regNames[8] = {
        "Increase Property Tax", "Reduce Loan Interest", "Housing Subsidy", "Luxury Property Tax",
        "Railway Modernization", "Electricity Tariff Revision", "Insurance Regulation", "Anti-Speculation Act"
    };
    int reg = rand() % 8;

    printf("Government Regulation\n%s\n", regNames[reg]);

    switch (reg) {
        case 0:
            currentIncomeTaxRate = currentIncomeTaxRate * 150 / 100;
            printf("Income Tax increases by 50%%.\n");
            break;
        case 1:
            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (players[i].hasLoan && players[i].loanInterestRate > 2) players[i].loanInterestRate -= 2;
            }
            printf("Interest decreases by 2%%.\n");
            break;
        case 2:
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY) board[i].houseCost = board[i].houseCost * 70 / 100;
            }
            printf("House construction costs reduce 30%%.\n");
            break;
        case 3:
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].hasHotel && board[i].owner != -1) {
                    int tax = board[i].price * 25 / 100;
                    players[board[i].owner].cash -= tax;
                    checkBankruptcy(&players[board[i].owner], board[i].owner, players, board);
                }
            }
            printf("Hotels incur a maintenance tax of 25%% of property value.\n");
            break;
        case 4:
            railwayRentMultiplier = railwayRentMultiplier * 125 / 100;
            printf("Railway rents increase 25%%.\n");
            break;
        case 5:
            utilityRentMultiplier = utilityRentMultiplier * 120 / 100;
            printf("Utility rents increase 20%%.\n");
            break;
        case 6:
            insurancePremiumMultiplier = insurancePremiumMultiplier * 85 / 100;
            printf("Insurance premiums decrease 15%%. Coverage remains unchanged.\n");
            break;
        case 7:
            antiSpeculationActive = 1;
            printf("Players may own at most three undeveloped properties.\n");
            break;
        } 
    }


void triggerEconomicEvent(struct Player players[], struct Property board[]) {
    char *eventNames[8] = {
        "Tourism Boom", "Fuel Crisis", "Heavy Monsoon", "Economic Recession",
        "Stock Market Boom", "Government Housing Programme", "Foreign Investment", "Political Unrest"
    };
    int event = rand() % 8;

    printf("Economic Event\n%s\n", eventNames[event]);

    switch (event) {
        case 0:   // Tourism Boom: Southern coastal properties +15% (Galle Fort, Unawatuna, Hikkaduwa - Yellow group)
            {
                int southern[3] = {26, 27, 29};
                for (int i = 0; i < 3; i++) {
                    board[southern[i]].price = board[southern[i]].price * 115 / 100;
                    board[southern[i]].baseRent = board[southern[i]].baseRent * 115 / 100;
                }
                printf("Southern coastal properties increase in value by 15%%.\n");
            }
            break;
        case 1:   // Fuel Crisis: railway rent doubles, development costs +20%
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == RAILWAY) board[i].price = board[i].price * 2;
                if (board[i].type == PROPERTY) {
                    board[i].houseCost = board[i].houseCost * 120 / 100;
                    board[i].hotelCost = board[i].hotelCost * 120 / 100;
                }
            }
            printf("Railway rent doubles. Development costs increase 20%%.\n");
            break;
        case 2:   // Heavy Monsoon: coastal properties -10%
            {
                int coastal[6] = {26, 27, 29, 8, 9, 34};   // Yellow + Light Blue coastal + Trincomalee
                for (int i = 0; i < 6; i++) {
                    board[coastal[i]].price = board[coastal[i]].price * 90 / 100;
                }
                printf("Coastal properties lose 10%% value.\n");
            }
            break;
        case 3:   // Economic Recession: property values -15%, rent -10%, loan interest +15%
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY) {
                    board[i].price = board[i].price * 85 / 100;
                    board[i].baseRent = board[i].baseRent * 90 / 100;
                }
            }
            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (players[i].hasLoan) players[i].loanInterestRate += 15;
            }
            printf("Property values decrease 15%%. Rent decreases 10%%. Loan interest increases 15%%.\n");
            break;
        case 4:   // Stock Market Boom: property values +10%, loan interest -10%
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY) board[i].price = board[i].price * 110 / 100;
            }
            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (players[i].hasLoan && players[i].loanInterestRate > 10) players[i].loanInterestRate -= 10;
            }
            printf("Property values increase 10%%. Loan interest decreases 10%%.\n");
            break;
        case 5:   // Government Housing Programme: house construction costs -25%
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY) board[i].houseCost = board[i].houseCost * 75 / 100;
            }
            printf("House construction costs reduce 25%%.\n");
            break;
        case 6:   // Foreign Investment: commercial properties +20% (interpreting as Orange/Red groups - business districts)
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY && (i >= 16 && i <= 24)) {
                    board[i].price = board[i].price * 120 / 100;
                }
            }
            printf("Commercial properties increase 20%%.\n");
            break;
        case 7:   // Political Unrest: hotel rent -50%
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].hasHotel) board[i].baseRent = board[i].baseRent * 50 / 100;
            }
            printf("Hotel occupancy decreases. Hotel rent drops by 50%%.\n");
            break;
    }
}

    
void printMarketConditions(int round) {
    char *groupNames[8] = {"Brown", "Light Blue", "Pink", "Orange", "Red", "Yellow", "Green", "Dark Blue"};
    char *cardNames[12] = {
        "Southern Tourism Boom", "Port City Expansion", "IT Industry Growth", "Northern Development Programme",
        "Tea Export Boom", "Airport Expansion", "University City Growth", "Beach Pollution",
        "Flood Damage", "Transport Strike", "Electricity Tariff Increase", "Water Shortage"
    };

    printf("=========================================\n");
    printf("Current Market Conditions\n");
    printf("=========================================\n");
    printf("Market Boom\n-------------\n");
    if (activeMarketBoomGroup != -1) {
        printf("%s (+25%%)\nRounds Remaining : %d\n", groupNames[activeMarketBoomGroup], activeMarketBoomEndRound - round);
    } else {
        printf("None\n");
    }
    printf("Market Decline\n----------------\n");
    if (activeMarketDeclineGroup != -1) {
        printf("%s (-20%%)\nRounds Remaining : %d\n", groupNames[activeMarketDeclineGroup], activeMarketDeclineEndRound - round);
    } else {
        printf("None\n");
    }
    printf("Regional Development\n-----------------------\n");
    if (activeRegionalCard != -1) {
        printf("%s\nRounds Remaining : %d\n", cardNames[activeRegionalCard], activeRegionalCardEndRound - round);
    } else {
        printf("None\n");
    }
    printf("Inflation\n------------\n%d%%\n", currentInflationRate);
    printf("Current Loan Interest\n-----------------------\n%d%%\n", 8 + (currentInflationRate > 0 ? currentInflationRate / 2 : 0));
    printf("=========================================\n\n");
}         

void applyRegionalDevelopmentCard(struct Property board[], int round) {
    if (activeRegionalCard != -1 && round >= activeRegionalCardEndRound) {
        activeRegionalCard = -1;   // reverts when period expires
    }

    if (activeRegionalCard == -1) {
        activeRegionalCard = rand() % 12;
        activeRegionalCardEndRound = round + 15;
    }

    char *cardNames[12] = {
        "Southern Tourism Boom", "Port City Expansion", "IT Industry Growth", "Northern Development Programme",
        "Tea Export Boom", "Airport Expansion", "University City Growth", "Beach Pollution",
        "Flood Damage", "Transport Strike", "Electricity Tariff Increase", "Water Shortage"
    };
    printf("Regional Development Card\n%s\n", cardNames[activeRegionalCard]);
}


