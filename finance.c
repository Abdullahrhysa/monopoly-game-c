#include <stdio.h>
#include "types.h"
#include <stdlib.h>

int countRailwaysOwned(struct Property board[], int ownerIndex) {
    int count = 0;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].type == RAILWAY && board[i].owner == ownerIndex) {
            count++;
        }
    }
    return count;
}

int countUtilitiesOwned(struct Property board[], int ownerIndex) {
    int count = 0;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].type == UTILITY && board[i].owner == ownerIndex) {
            count++;
        }
    }
    return count;
}

int calculateRent(struct Property board[], struct Player players[], struct Property *square, int diceRoll) {
    if (square->isMortgaged) return 0;
    if (square->isDamaged) return 0;

    int ownerCard = (square->owner != -1) ? players[square->owner].eventCard : -1;

    if (square->type == PROPERTY) {
        int rent;
        if (square->hasHotel) rent = square->baseRent * 10;
        else if (square->houses == 4) rent = square->baseRent * 7;
        else if (square->houses == 3) rent = square->baseRent * 5;
        else if (square->houses == 2) rent = square->baseRent * 3;
        else if (square->houses == 1) rent = square->baseRent * 2;
        else rent = square->baseRent * 1;

        if (square->houses > 0 || square->hasHotel) {
            int conditionPercent;
            if (square->buildingCondition >= 90) conditionPercent = 100;
            else if (square->buildingCondition >= 75) conditionPercent = 90;
            else if (square->buildingCondition >= 50) conditionPercent = 75;
            else if (square->buildingCondition >= 25) conditionPercent = 50;
            else conditionPercent = 0;
            rent = rent * conditionPercent / 100;
            if (square->structuralDamage) rent = rent * 75 / 100;
        }

        if (square->hasHotel && ownerCard == 0) rent = rent * 2;
        if (square->hasHotel && ownerCard == 13) rent = rent * 150 / 100;

        int group = getGroupIndex(square - board);
        if (group == activeMarketBoomGroup) rent = rent * 125 / 100;
        if (group == activeMarketDeclineGroup) rent = rent * 80 / 100;

        int idx = square - board;
        if (activeRegionalCard == 0 && (idx == 26 || idx == 27 || idx == 29)) rent = rent * 140 / 100;
        if (activeRegionalCard == 5 && (idx == 16 || idx == 18 || idx == 19)) rent = rent * 130 / 100;
        if (activeRegionalCard == 7 && (idx == 26 || idx == 27 || idx == 29)) rent = rent * 70 / 100;

        rent = rent * inflationMultiplier / 100;
        return rent;
    }

    if (square->type == RAILWAY) {
        int owned = countRailwaysOwned(board, square->owner);
        int baseRailwayRent;
        if (owned == 1) baseRailwayRent = 250;
        else if (owned == 2) baseRailwayRent = 500;
        else if (owned == 3) baseRailwayRent = 1000;
        else baseRailwayRent = 2000;
        int rent = baseRailwayRent * railwayRentMultiplier / 100;
        if (ownerCard == 1) rent = rent * 2;
        return rent;
    }

    if (square->type == UTILITY) {
        int owned = countUtilitiesOwned(board, square->owner);
        int baseUtilityRent = (owned == 2) ? diceRoll * 10 : diceRoll * 4;
        int rent = baseUtilityRent * utilityRentMultiplier / 100;
        if (ownerCard == 10) rent = rent / 2;
        return rent;
    }

    return 0;
}
void payRent(struct Player players[], int payerIndex, struct Property board[], struct Property *square, int diceRoll) {
    int rent = calculateRent(board, players, square, diceRoll);
    int ownerIndex = square->owner;

    players[payerIndex].cash -= rent;
    players[ownerIndex].cash += rent;

    printf("%s landed on %s.\n", players[payerIndex].name, square->name);
    printf("Rent Paid : LKR %d.\n", rent);
    printf("Owner : %s.\n", players[ownerIndex].name);
}

int getMaxBid(struct Player *player, struct Property *square) {
    switch (player->strategy) {
        case AGGRESSIVE:    return square->price * 12 / 10;  // up to 120% - Section 3.1
        case CONSERVATIVE:  return square->price;             // only below market value - 3.2
        case RISK_TAKER:    return player->cash;              // until cash exhausted - 3.3
        case OPPORTUNISTIC: return square->price * 8 / 10;    // prefers discount - 3.4
        default: return 0;
    }
}

void runAuction(struct Player players[], struct Property board[], struct Property *square) {
    int active[NUM_PLAYERS];
    int activeCount = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        active[i] = !players[i].isBankrupt;
        if (active[i]) activeCount++;
    }

    int currentBid = square->price / 2;   // opening bid = 50% market value
    int highestBidder = -1;

    printf("Auction Started.\n");
    printf("Property :\n%s\n", square->name);
    printf("Opening Bid :\nLKR %d.\n", currentBid);

    while (activeCount > 1) {
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (!active[i]) continue;

            int nextBid = currentBid + 250;   //  min increment 250
            int maxWilling = getMaxBid(&players[i], square);

            if (nextBid <= maxWilling && nextBid <= players[i].cash) {
                currentBid = nextBid;
                highestBidder = i;
                printf("%s bids LKR %d.\n", players[i].name, currentBid);
            } else {
                active[i] = 0;   // withdraws permanently
                activeCount--;
                printf("%s withdraws.\n", players[i].name);
            }

            if (activeCount <= 1) break;
        }
    }

    if (highestBidder != -1) {
        players[highestBidder].cash -= currentBid;
        square->owner = highestBidder;
        int idx = square - board;   // pointer arithmetic: recover board index
        players[highestBidder].ownedSquares[idx] = 1;
        printf("%s wins the auction.\n", players[highestBidder].name);
    } else {
        printf("No bids received. Property remains with the Bank.\n");   
    }
}

void declareBankrupt(struct Player *player, int playerIndex, struct Player players[], struct Property board[]) {
    player->isBankrupt = 1;
    printf("%s has been declared bankrupt.\n", player->name);

    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex) {
            board[i].houses = 0;
            board[i].hasHotel = 0;
            board[i].isMortgaged = 0;
            board[i].isLoanLocked = 0;
            board[i].insuranceType = 0;
            player->ownedSquares[i] = 0;
            board[i].owner = -1;

            if (board[i].type == PROPERTY || board[i].type == RAILWAY || board[i].type == UTILITY) {
                runAuction(players, board, &board[i]);
            }
        }
    }

    player->hasLoan = 0;
    player->loanAmount = 0;
    player->loanRoundsLeft = 0;

    printf("Remaining assets transferred to the Bank.\n");
}

void checkBankruptcy(struct Player *player, int playerIndex, struct Player players[], struct Property board[]) {
    if (player->isBankrupt) return;

    if (player->cash < 0) {
        // Last resort: try mortgaging owned properties before declaring bankruptcy
        for (int i = 0; i < NUM_SQUARES && player->cash < 0; i++) {
            if (board[i].owner == playerIndex && !board[i].isMortgaged && !board[i].isLoanLocked &&
                (board[i].type == PROPERTY || board[i].type == RAILWAY || board[i].type == UTILITY)) {
                mortgageProperty(player, &board[i]);
            }
        }
    }

    if (player->cash < 0) {
    declareBankrupt(player, playerIndex, players, board);
    }
}
int calculateMaxLoan(struct Player *player, int playerIndex, struct Property board[]) {
    int totalMortgageValue = 0;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex && !board[i].isMortgaged &&
            (board[i].type == PROPERTY || board[i].type == RAILWAY || board[i].type == UTILITY)) {
            totalMortgageValue += board[i].mortgageValue;
        }
    }
    return totalMortgageValue * 75 / 100;   
}
void takeLoan(struct Player *player, int playerIndex, struct Property board[], int amount) {
    player->hasLoan = 1;
    player->loanAmount = amount;
    player->loanInterestRate = 8 + (currentInflationRate > 0 ? currentInflationRate / 2 : 0);   // simplified inflation-adjustment for new loans only
    player->loanRoundsLeft = 20;    
    player->cash += amount;

    printf("%s obtained a secured loan.\n", player->name);
    printf("Loan Amount : LKR %d.\n", amount);
    printf("Collateral :\n");
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex && !board[i].isMortgaged &&
            (board[i].type == PROPERTY || board[i].type == RAILWAY || board[i].type == UTILITY)) {
            board[i].isLoanLocked = 1;
            printf("%s\n", board[i].name);
        }
    }
    printf("Interest Rate : %d%%\n", player->loanInterestRate);
    printf("Duration : 20 Rounds\n");
}

void repayLoan(struct Player *player, int amount) {
    if (amount > player->loanAmount) amount = player->loanAmount;
    player->cash -= amount;
    player->loanAmount -= amount;

    printf("%s repaid LKR %d.\n", player->name, amount);
    printf("Outstanding Balance :\nLKR %d.\n", player->loanAmount);

    if (player->loanAmount <= 0) {
        player->hasLoan = 0;
        player->loanRoundsLeft = 0;
    }
}

void handleBankVisit(struct Player *player, int playerIndex, struct Property board[]) {
    printf("%s landed on Bank of Ceylon.\n", player->name);

    // Attempt to un-mortgage properties if cash allows (applies to all strategies)
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex && board[i].isMortgaged) {
            int repayAmount = board[i].mortgageValue * 110 / 100;
            if (player->cash > repayAmount * 2) {   // only if comfortably affordable
                unmortgageProperty(player, &board[i]);
            }
        }
    }

    int maxLoan = calculateMaxLoan(player, playerIndex, board);
    switch (player->strategy) {
        case AGGRESSIVE:   // borrows to boost rental income; repays once excess cash > 2x loan
            if (!player->hasLoan && maxLoan > 0) {
                takeLoan(player, playerIndex, board, maxLoan);
            } else if (player->hasLoan && player->cash > player->loanAmount * 2) {
                repayLoan(player, player->loanAmount);
            }
            break;
        case CONSERVATIVE:   //avoids loans; repays immediately when able
            if (player->hasLoan && player->cash > player->loanAmount) {
                repayLoan(player, player->loanAmount);
            }
            break;
        case RISK_TAKER:
            if (!player->hasLoan && maxLoan > 0) {
                takeLoan(player, playerIndex, board, maxLoan);
            } else if (player->hasLoan) {
                increaseLoan(player, playerIndex, board);
                if (player->loanRoundsLeft < 10) refinanceLoan(player);
            }
            break;
        case OPPORTUNISTIC:   // borrows only if return justifies it (simplified: worthwhile min amount)
            if (!player->hasLoan && maxLoan >= 2000) {
                takeLoan(player, playerIndex, board, maxLoan);
            } else if (player->hasLoan && player->cash > player->loanAmount * 3 / 2) {
                repayLoan(player, player->loanAmount / 2);
            }
            break;
    }
}

int countPlayerProperties(struct Player *player) {
    int count = 0;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (player->ownedSquares[i]) count++;
    }
    return count;
}

int countPlayerHotels(struct Property board[], int playerIndex) {
    int count = 0;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex && board[i].hasHotel) count++;
    }
    return count;
}

// Net Worth = Cash + Property Value + Building Value - Outstanding Loans
int calculateNetWorth(struct Player *player, int playerIndex, struct Property board[]) {
    int worth = player->cash;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex) {
            worth += board[i].price;
            worth += board[i].houses * board[i].houseCost;
            if (board[i].hasHotel) worth += board[i].hotelCost;
        }
    }
    worth -= player->loanAmount;
    return worth;
}

void foreclosureLoan(struct Player *player, int playerIndex, struct Player players[], struct Property board[]) {
    printf("%s has defaulted.\n", player->name);
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex && board[i].isLoanLocked) {
            board[i].owner = -1;
            board[i].houses = 0;
            board[i].hasHotel = 0;
            board[i].isMortgaged = 0;
            board[i].isLoanLocked = 0;
            board[i].insuranceType = 0;
            player->ownedSquares[i] = 0;
        }
    }
    printf("Collateral has been foreclosed.\n");
    printf("Outstanding debt cleared.\n");
    player->hasLoan = 0;
    player->loanAmount = 0;
    player->loanRoundsLeft = 0;

    if (countPlayerProperties(player) == 0 && player->cash <= 0) {
        checkBankruptcy(player, playerIndex, players, board);   
    }
}

void processLoans(struct Player players[], struct Property board[]) {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (players[i].isBankrupt || !players[i].hasLoan) continue;

        players[i].loanAmount += players[i].loanAmount * players[i].loanInterestRate / 100;  
        players[i].loanRoundsLeft--;

        if (players[i].loanRoundsLeft <= 0) {
            foreclosureLoan(&players[i], i, players, board);
        }
    }
}

// Sums only the purchase price of owned properties - explicitly excludes building value,
// per the Community Development Fund clarification ("current market rate of only the property, not buildings")
int calculatePropertyValue(struct Player *player, int playerIndex, struct Property board[]) {
    int total = 0;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex) {
            total += board[i].price;
        }
    }
    return total;
}

void payIncomeTax(struct Player *player, int playerIndex, struct Player players[], struct Property board[]) {
    int tax = player->cash * currentIncomeTaxRate / 100;
    player->cash -= tax;
    printf("%s landed on Income Tax.\n", player->name);
    printf("Tax Paid : LKR %d.\n", tax);
    printf("Remaining Balance : LKR %d.\n", player->cash);
    checkBankruptcy(player, playerIndex, players, board);
}

void payCommunityDevelopmentFund(struct Player *player, int playerIndex, struct Player players[], struct Property board[]) {
    int propertyValue = calculatePropertyValue(player, playerIndex, board);
    int tax = propertyValue * 10 / 100;   // documented assumption per clarification
    player->cash -= tax;
    printf("%s landed on Community Development Fund.\n", player->name);
    printf("Tax Paid : LKR %d.\n", tax);
    printf("Remaining Balance : LKR %d.\n", player->cash);
    checkBankruptcy(player, playerIndex, players, board);
}

// Finds one of the player's owned, developed properties without active insurance - simple target-picker
struct Property* findInsurableProperty(struct Property board[], int playerIndex, int requireHotel) {
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex && board[i].type == PROPERTY &&
            board[i].insuranceType == 0) {
            if (requireHotel && !board[i].hasHotel) continue;
            if (!requireHotel && (board[i].houses > 0 || board[i].hasHotel)) return &board[i];
        }
    }
    return NULL;
}

void purchaseInsurance(struct Player *player, struct Property *target, int policyType) {
    int premiumPercent;
    if (policyType == 1) premiumPercent = 5;
    else if (policyType == 2) premiumPercent = 10;
    else premiumPercent = 15;

    int premium = target->price * premiumPercent / 100 * inflationMultiplier / 100;
    if (player->eventCard == 15) premium = premium * 80 / 100;   // Insurance Discount

    player->cash -= premium;
    target->insuranceType = policyType;
    target->insuranceRoundsLeft = 20;

    const char *policyName = (policyType == 1) ? "Basic Property Insurance" :
                               (policyType == 2) ? "Comprehensive Insurance" : "Business Interruption Insurance";
    printf("%s purchased.\n", policyName);
    printf("Property : %s\n", target->name);
    printf("Premium : LKR %d.\n", premium);
}

void handleInsuranceVisit(struct Player *player, int playerIndex, struct Property board[]) {
    printf("%s landed on an Insurance square.\n", player->name);
    struct Property *target;

    switch (player->strategy) {
        case AGGRESSIVE:   //Basic for houses, Comprehensive for hotels
            target = findInsurableProperty(board, playerIndex, 1);   // hotel first
            if (target != NULL && player->cash >= target->price * 10 / 100) {
                purchaseInsurance(player, target, 2);
                break;
            }
            target = findInsurableProperty(board, playerIndex, 0);   // then houses
            if (target != NULL && player->cash >= target->price * 5 / 100) {
                purchaseInsurance(player, target, 1);
            }
            break;
        case CONSERVATIVE:   // Comprehensive for every developed property
            target = findInsurableProperty(board, playerIndex, 0);
            if (target == NULL) target = findInsurableProperty(board, playerIndex, 1);
            if (target != NULL && player->cash >= target->price * 10 / 100) {
                purchaseInsurance(player, target, 2);
            }
            break;
       case RISK_TAKER:   // purchases only after experiencing a financial loss
            {
                int hadLoss = 0;
                for (int i = 0; i < NUM_SQUARES; i++) {
                    if (board[i].owner == playerIndex && board[i].repairCost > 0 && board[i].insuranceType == 0) {
                        hadLoss = 1;
                    }
                }
                if (hadLoss) {
                    target = findInsurableProperty(board, playerIndex, 0);
                    if (target == NULL) target = findInsurableProperty(board, playerIndex, 1);
                    if (target != NULL && player->cash >= target->price * 5 / 100) {
                        purchaseInsurance(player, target, 1);
                    }
                }
            }
            break;
        case OPPORTUNISTIC:   // 3.4: Comprehensive only for high-value developments
            target = findInsurableProperty(board, playerIndex, 0);
            if (target == NULL) target = findInsurableProperty(board, playerIndex, 1);
            if (target != NULL && target->price >= 6000 && player->cash >= target->price * 10 / 100) {
                purchaseInsurance(player, target, 2);
            }
            break;
    }
}

void mortgageProperty(struct Player *player, struct Property *property) {
    if (property->isMortgaged) {
        printf("%s is already mortgaged.\n", property->name);
        return;
    }
    if (property->isLoanLocked) {
        printf("%s cannot be mortgaged - already pledged as loan collateral.\n", property->name);
        return;
    }
    property->isMortgaged = 1;
    player->cash += property->mortgageValue;
    printf("%s mortgaged %s for LKR %d.\n", player->name, property->name, property->mortgageValue);
}

void unmortgageProperty(struct Player *player, struct Property *property) {
    if (!property->isMortgaged) {
        printf("%s is not mortgaged.\n", property->name);
        return;
    }
    int repayAmount = property->mortgageValue * 110 / 100;   // assumption: 10% interest to un-mortgage, standard convention
    if (player->cash < repayAmount) {
        printf("%s cannot afford to un-mortgage %s.\n", player->name, property->name);
        return;
    }
    property->isMortgaged = 0;
    player->cash -= repayAmount;
    printf("%s un-mortgaged %s for LKR %d.\n", player->name, property->name, repayAmount);
}
void triggerDisaster(struct Player players[], struct Property board[]) {
    char *disasters[5] = {"Fire", "Flood", "Riot", "Building Collapse", "Electrical Failure"};

    // Collect all developed properties (houses or hotel) into a candidate list
    int candidates[NUM_SQUARES];
    int count = 0;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].type == PROPERTY && board[i].owner != -1 &&
            (board[i].houses > 0 || board[i].hasHotel) && !board[i].isDamaged) {
            candidates[count++] = i;
        }
    }
    if (count == 0) return;   // nothing developed to damage

    int chosen = candidates[rand() % count];
    struct Property *p = &board[chosen];
    char *disasterName = disasters[rand() % 5];

    p->repairCost = (p->hasHotel ? p->hotelCost / 2 : p->houseCost * (p->houses > 0 ? p->houses : 1) / 2) * inflationMultiplier / 100;
    p->isDamaged = 1;

    printf("%s occurred.\n", disasterName);
    printf("Affected Property :\n%s.\n", p->name);

    int ownerIdx = p->owner;
    if (p->insuranceType != 0) {
        int compensationPercent = (p->insuranceType == 1) ? 80 : 100;   // Basic 80%, Comprehensive/Business 100%
        int compensation = p->repairCost * compensationPercent / 100;
        players[ownerIdx].cash += compensation;
        printf("Insurance Claim Approved.\n");
        printf("Compensation Paid :\nLKR %d.\n", compensation);
        p->isDamaged = 0;   // repaired immediately via claim
        p->repairCost = 0;
    } else {
        printf("%s is uninsured. Full repair cost is owed.\n", players[ownerIdx].name);
    }

    checkBankruptcy(&players[ownerIdx], ownerIdx, players, board);
}

// automatic repair once owner has sufficient funds
void attemptAutoRepair(struct Player *player, int playerIndex, struct Player players[], struct Property board[]) {
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex && board[i].isDamaged && player->cash >= board[i].repairCost) {
            player->cash -= board[i].repairCost;
            printf("%s repaired %s for LKR %d.\n", player->name, board[i].name, board[i].repairCost);
            board[i].isDamaged = 0;
            board[i].repairCost = 0;
        }
    }
}

void renovateProperty(struct Player *player, struct Property *property) {
    int cost;
    if (property->structuralDamage) {
        // renovating damaged buildings costs 25% of replacement value
        int replacementValue = property->hasHotel ? property->hotelCost : property->houseCost * property->houses;
        cost = replacementValue * 25 / 100;
    } else {
        cost = property->price * 10 / 100;   // standard depreciation renovation
    }

    if (player->cash < cost) return;
    player->cash -= cost;

    property->age = 0;
    property->buildingCondition = 100;
    property->roundsNeglected = 0;
    if (property->structuralDamage) {
        property->structuralDamage = 0;
        property->price = property->price * 100 / 85;   // reverse the earlier 15% structural-damage price cut
    }

    printf("%s renovated %s.\n", player->name, property->name);
    printf("Renovation Cost : LKR %d.\n", cost);
}

void maintainBuildings(struct Player *player, int playerIndex, struct Property board[]) {
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (board[i].owner == playerIndex && (board[i].houses > 0 || board[i].hasHotel) &&
            board[i].buildingCondition < 100) {
            int cost = board[i].hasHotel ? board[i].hotelCost * 8 / 100 : board[i].houseCost * 5 / 100 * board[i].houses;
            if (board[i].structuralDamage) cost = cost * 150 / 100;   // Rule-LK 28: future maintenance costs +50%
            cost = cost * inflationMultiplier / 100;

            if (player->cash >= cost) {
                player->cash -= cost;
                board[i].buildingCondition = 100;
                board[i].roundsNeglected = 0;
                printf("%s performed maintenance on %s.\n", player->name, board[i].name);
                printf("Maintenance Cost : LKR %d.\n", cost);
            }
        }
    }
}

void refinanceLoan(struct Player *player) {
    player->loanRoundsLeft = 20;
    player->loanInterestRate = 8 + (currentInflationRate > 0 ? currentInflationRate / 2 : 0);
    printf("%s refinanced their loan.\n", player->name);
    printf("New Duration : 20 Rounds.\n");
    printf("New Interest Rate : %d%%\n", player->loanInterestRate);
}

void increaseLoan(struct Player *player, int playerIndex, struct Property board[]) {
    int maxLoan = calculateMaxLoan(player, playerIndex, board);
    int additional = maxLoan - player->loanAmount;
    if (additional <= 0) return;
    player->loanAmount += additional;
    player->cash += additional;
    printf("%s increased their loan by LKR %d.\n", player->name, additional);
    printf("New Loan Amount : LKR %d.\n", player->loanAmount);
}

void drawEventCard(struct Player *player, int playerIndex, struct Player players[], struct Property board[], int round) {
    static int deckPosition = 0;

    char *cardNames[20] = {
        "Tourism Hype", "Fuel Shortage", "Heavy Floods", "Political Rally",
        "Stock Market Rise", "Economic Downturn", "Housing Subsidy", "Interest Rate Cut",
        "Interest Rate Increase", "Tax Amnesty", "Power Failure", "Foreign Funding",
        "Port Expansion", "Festival Season", "Labour Strike", "Insurance Discount",
        "Property Revaluation", "Currency Depreciation", "Government Grant", "National Disaster"
    };

    int card = deckPosition;
    deckPosition = (deckPosition + 1) % 20;

    printf("Economic Event\n%s\n", cardNames[card]);

    switch (card) {
        case 0:   // Tourism Hype: hotels earn double rent, applies to drawing player for 15 rounds
        case 1:   // Fuel Shortage: railway rent doubles, 15 rounds
        case 10:  // Power Failure: utility income halved, 15 rounds
        case 13:  // Festival Season: hotels +50% rent, 15 rounds
        case 15:  // Insurance Discount: premiums -20%, 15 rounds
            player->eventCard = card;
            player->eventCardEndRound = round + 15;
            printf("This effect applies to %s for 15 rounds.\n", player->name);
            break;
        case 2:
            {
                int coastal[6] = {26, 27, 29, 8, 9, 34};
                int candidates[6], count = 0;
                for (int i = 0; i < 6; i++) {
                    if (board[coastal[i]].owner != -1 && (board[coastal[i]].houses > 0 || board[coastal[i]].hasHotel) && !board[coastal[i]].isDamaged) {
                        candidates[count++] = coastal[i];
                    }
                }
                if (count > 0) {
                    int chosen = candidates[rand() % count];
                    struct Property *p = &board[chosen];
                    p->repairCost = p->hasHotel ? p->hotelCost / 2 : p->houseCost * (p->houses > 0 ? p->houses : 1) / 2;
                    p->isDamaged = 1;
                    printf("Flooding damaged %s.\n", p->name);
                }
            }
            break;
        case 3:
            {
                int owned[NUM_SQUARES], count = 0;
                for (int i = 0; i < NUM_SQUARES; i++) {
                    if (board[i].type == PROPERTY && board[i].owner != -1 && !board[i].isDamaged) owned[count++] = i;
                }
                if (count > 0) {
                    int chosen = owned[rand() % count];
                    board[chosen].isDamaged = 1;
                    board[chosen].repairCost = 200;
                    printf("%s has been closed temporarily.\n", board[chosen].name);
                }
            }
            break;
        case 4:
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY) board[i].price = board[i].price * 110 / 100;
            }
            printf("All property values increase by 10%%.\n");
            break;
        case 5:
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY) board[i].price = board[i].price * 85 / 100;
            }
            printf("Property values decrease by 15%%.\n");
            break;
        case 6:
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY) board[i].houseCost = board[i].houseCost * 70 / 100;
            }
            printf("House construction cost reduced by 30%%.\n");
            break;
        case 7:
            if (player->hasLoan && player->loanInterestRate > 2) {
                player->loanInterestRate -= 2;
                printf("%s's loan interest reduced by 2%%.\n", player->name);
            }
            break;
        case 8:
            if (player->hasLoan) {
                player->loanInterestRate += 2;
                printf("%s's loan interest increased by 2%%.\n", player->name);
            }
            break;
        case 9:
            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (!players[i].isBankrupt) players[i].cash += 2000;
            }
            printf("Each player receives LKR 2000.\n");
            break;
        case 11:
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY && i >= 16 && i <= 24) board[i].price = board[i].price * 115 / 100;
            }
            printf("Commercial property values increase by 15%%.\n");
            break;
        case 12:
            railwayRentMultiplier = railwayRentMultiplier * 120 / 100;
            printf("Railway station values increase by 20%%.\n");
            break;
        case 14:
            printf("Construction suspended for 2 rounds.\n");
            break;
        case 16:
            {
                int colorGroups[8][3] = {
                    {1, 3, -1}, {6, 8, 9}, {11, 13, 14}, {16, 18, 19},
                    {21, 23, 24}, {26, 27, 29}, {31, 32, 34}, {37, 39, -1}
                };
                int groupSizes[8] = {2, 3, 3, 3, 3, 3, 3, 2};
                int g = rand() % 8;
                for (int j = 0; j < groupSizes[g]; j++) {
                    if (colorGroups[g][j] != -1) board[colorGroups[g][j]].price = board[colorGroups[g][j]].price * 115 / 100;
                }
                printf("A random property group appreciates by 15%%.\n");
            }
            break;
        case 17:
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (board[i].type == PROPERTY) {
                    board[i].houseCost = board[i].houseCost * 110 / 100;
                    board[i].hotelCost = board[i].hotelCost * 110 / 100;
                }
            }
            printf("Construction costs increase by 10%%.\n");
            break;
        case 18:
            {
                int lucky = rand() % NUM_PLAYERS;
                players[lucky].cash += 5000;
                printf("%s receives LKR 5000.\n", players[lucky].name);
            }
            break;
        case 19:
            triggerDisaster(players, board);
            break;
    }

    checkBankruptcy(player, playerIndex, players, board);
}