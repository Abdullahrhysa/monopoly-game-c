#ifndef TYPES_H
#define TYPES_H

#define NUM_SQUARES 40
#define NUM_PLAYERS 4
#define STARTING_CASH 30000
#define MAX_ROUNDS 1000

enum SquareType {PROPERTY, RAILWAY, UTILITY, BANK, INSURANCE, TAX, EVENT, SPECIAL };
enum Strategy { AGGRESSIVE, CONSERVATIVE, RISK_TAKER, OPPORTUNISTIC };

struct Property {
    char name[50];
    enum SquareType type;
    int price;
    int mortgageValue;
    int baseRent;
    int houseCost;
    int hotelCost;
    int owner;          // -1 = Bank
    int isMortgaged; 
    int isLoanLocked;   // 1 if pledged as loan collateral, 0 otherwise    // 0 = no, 1 = yes
    int isDamaged;      // 0 = fine, 1 = damaged (Rule-LK 11 - can't collect rent)
    int repairCost;  
    int buildingCondition;   // 100 down to 0, Rule-LK 25
    int roundsNeglected;     // consecutive rounds without maintenance
    int structuralDamage;    // 0/1, set once neglect exceeds 20 rounds  // cost to fix, set when disaster strikes   
    int insuranceType;  
    int insuranceRoundsLeft;   // tracks 20-round policy duration // 0=none, 1=basic, 2=comprehensive, 3=business
    int houses;          // 0-4
    int hasHotel;        // 0 = no, 1 = yess
    int age;     
             
};

struct Player {
    char name[50];
    enum Strategy strategy;
    int cash;
    int position;                     // current square index 0-39
    int ownedSquares[NUM_SQUARES];    // 1 if owns square i, 0 otherwise
    int hasLoan;
    int loanAmount;
    int loanInterestRate;    // as a whole number percent, e.g. 8
    int loanRoundsLeft;
    int inJail;
    int jailTurnsLeft;
    int eventCard;          // -1 = none active, else 0-19 (index into the 20 National Event Cards)
    int eventCardEndRound;  // round number when this player's card effect expires
    int isBankrupt;
};

extern int activeMarketBoomGroup;
extern int activeMarketBoomEndRound;
extern int activeMarketDeclineGroup;
extern int activeMarketDeclineEndRound;
extern int activeRegionalCard;
extern int activeRegionalCardEndRound;
extern int currentInflationRate;
extern int railwayRentMultiplier;
extern int utilityRentMultiplier;
extern int currentIncomeTaxRate;
extern int inflationMultiplier;
extern int antiSpeculationActive;
extern int insurancePremiumMultiplier;

void initBoard(struct Property board[]);
void initPlayers(struct Player players[]);
char* strategyName(enum Strategy s);
int rollDice(void);
void determineTurnOrder(struct Player players[], int order[]);

void movePlayer(struct Player *player, int diceRoll);
void sendToJail(struct Player *player);
int estimateRent(struct Property *square);
int shouldBuy(struct Player *player, int playerIndex, struct Property board[], struct Property *square);
void buyProperty(struct Player *player, int playerIndex, struct Property *square);
int countRailwaysOwned(struct Property board[], int ownerIndex);
int countUtilitiesOwned(struct Property board[], int ownerIndex);
int calculateRent(struct Property board[], struct Player players[], struct Property *square, int diceRoll);void payRent(struct Player players[], int payerIndex, struct Property board[], struct Property *square, int diceRoll);
void playTurn(struct Player *player, int playerIndex, struct Player players[], struct Property board[], int round);
void handleLanding(struct Player *player, int playerIndex, struct Player players[], struct Property board[], int diceRoll, int round);
void drawEventCard(struct Player *player, int playerIndex, struct Player players[], struct Property board[], int round);

int getMaxBid(struct Player *player, struct Property *square);
void runAuction(struct Player players[], struct Property board[], struct Property *square);

int getGroupIndex(int squareIndex);
int hasMonopoly(struct Property board[], int ownerIndex, int groupIndex);
void buildOnGroup(struct Player *player, int playerIndex, struct Property board[], int groupIndex);
void constructBuildings(struct Player *player, int playerIndex, struct Property board[]);

void declareBankrupt(struct Player *player, int playerIndex, struct Player players[], struct Property board[]);
void checkBankruptcy(struct Player *player, int playerIndex, struct Player players[], struct Property board[]);

int calculateMaxLoan(struct Player *player, int playerIndex, struct Property board[]);
void takeLoan(struct Player *player, int playerIndex, struct Property board[], int amount);
void repayLoan(struct Player *player, int amount);
void handleBankVisit(struct Player *player, int playerIndex, struct Property board[]);

int countPlayerProperties(struct Player *player);
int countPlayerHotels(struct Property board[], int playerIndex);
int calculateNetWorth(struct Player *player, int playerIndex, struct Property board[]);
void foreclosureLoan(struct Player *player, int playerIndex, struct Player players[], struct Property board[]);
void processLoans(struct Player players[], struct Property board[]);
void printRoundSummary(int round, struct Player players[], struct Property board[]);
int countSolventPlayers(struct Player players[]);
void playGame(struct Player players[], struct Property board[], int order[]);

int calculatePropertyValue(struct Player *player, int playerIndex, struct Property board[]);
void payIncomeTax(struct Player *player, int playerIndex, struct Player players[], struct Property board[]);
void payCommunityDevelopmentFund(struct Player *player, int playerIndex, struct Player players[], struct Property board[]);

struct Property* findInsurableProperty(struct Property board[], int playerIndex, int requireHotel);
void purchaseInsurance(struct Player *player, struct Property *target, int policyType);
void handleInsuranceVisit(struct Player *player, int playerIndex, struct Property board[]);

void applyPropertyDepreciation(struct Property board[], int round);
void reviewPropertyMarket(struct Property board[], int round);
void triggerEconomicEvent(struct Player players[], struct Property board[]);
void triggerGovernmentRegulation(struct Player players[], struct Property board[]);

void mortgageProperty(struct Player *player, struct Property *property);
void unmortgageProperty(struct Player *player, struct Property *property);

void triggerDisaster(struct Player players[], struct Property board[]);
void attemptAutoRepair(struct Player *player, int playerIndex, struct Player players[], struct Property board[]);
void renovateProperty(struct Player *player, struct Property *property);

void applyRegionalDevelopmentCard(struct Property board[], int round);
void printMarketConditions(int round);

void applyInflation(int round);
void decayBuildingCondition(struct Property board[]);
void maintainBuildings(struct Player *player, int playerIndex, struct Property board[]);

void refinanceLoan(struct Player *player);
void increaseLoan(struct Player *player, int playerIndex, struct Property board[]);

void checkEventCardExpiry(struct Player players[], int round);
int countUndevelopedProperties(struct Property board[], int playerIndex);

void updateInsurancePolicies(struct Property board[], struct Player players[]);
#endif