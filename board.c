#include <string.h>
#include "types.h"

void setProperty(struct Property *p, char name[], int price, int mortgage,
                  int rent, int houseCost, int hotelCost) {
    strcpy(p->name, name);
    p->type = PROPERTY;
    p->price = price;
    p->mortgageValue = mortgage;
    p->baseRent = rent;
    p->houseCost = houseCost;
    p->hotelCost = hotelCost;
    p->owner = -1;
    p->isMortgaged = 0;
    p->isLoanLocked = 0;
    p->isDamaged = 0;
    p->repairCost = 0;
    p->buildingCondition = 100;
    p->roundsNeglected = 0;
    p->structuralDamage = 0;
    p->insuranceType = 0;
    p->insuranceRoundsLeft = 0;
    p->houses = 0;
    p->hasHotel = 0;
    p->age = 0;
}

void setSquare(struct Property *p, char name[], enum SquareType type) {
    strcpy(p->name, name);
    p->type = type;
    p->price = 0;
    p->mortgageValue = 0;
    p->baseRent = 0;
    p->houseCost = 0;
    p->hotelCost = 0;
    p->owner = -1;
    p->isMortgaged = 0;
    p->isLoanLocked = 0;
    p->isDamaged = 0;
    p->repairCost = 0;
    p->buildingCondition = 100;
    p->roundsNeglected = 0;
    p->structuralDamage = 0;
    p->insuranceType = 0;
    p->insuranceRoundsLeft = 0;
    p->houses = 0;
    p->hasHotel = 0;
    p->age = 0;
}

void initBoard(struct Property board[]) {
    setSquare(&board[0], "GO", SPECIAL);
    setProperty(&board[1], "Pettah", 1500, 750, 100, 500, 2000);
    setSquare(&board[2], "Community Development Fund", EVENT);
    setProperty(&board[3], "Maradana", 1800, 750, 120, 500, 2000);
    setSquare(&board[4], "Income Tax", TAX);
    setSquare(&board[5], "Colombo Fort Railway Station", RAILWAY);
    board[5].price = 2000;
    board[5].mortgageValue = 1000;
    setProperty(&board[6], "Bambalapitiya", 2500, 1250, 180, 750, 3000);
    setSquare(&board[7], "National Event Card", EVENT);
    setProperty(&board[8], "Wellawatte", 2700, 1250, 200, 750, 3000);
    setProperty(&board[9], "Mount Lavinia", 3000, 1250, 220, 750, 3000);
    setSquare(&board[10], "Jail / Just Visiting", SPECIAL);
    setProperty(&board[11], "Nugegoda", 3500, 1750, 260, 1000, 4000);
    setSquare(&board[12], "Ceylon Electricity Board", UTILITY);
    board[12].price = 1500;
    board[12].mortgageValue = 750;
    setProperty(&board[13], "Maharagama", 3800, 1750, 280, 1000, 4000);
    setProperty(&board[14], "Kottawa", 4000, 1750, 300, 1000, 4000);
    setSquare(&board[15], "Kandy Railway Station", RAILWAY);
    board[15].price = 2000;
    board[15].mortgageValue = 1000;     
    setProperty(&board[16], "Negombo", 4500, 2250, 350, 1250, 5000);
    setSquare(&board[17], "Sri Lanka Insurance", INSURANCE);
    setProperty(&board[18], "Katunayake", 4700, 2250, 370, 1250, 5000);
    setProperty(&board[19], "Ja-Ela", 5000, 2250, 400, 1250, 5000);
    setSquare(&board[20], "Free Parking", SPECIAL);
    setProperty(&board[21], "Kandy City", 5500, 2750, 450, 1500, 6000);
    setSquare(&board[22], "National Event Card", EVENT);
    setProperty(&board[23], "Peradeniya", 5800, 2750, 480, 1500, 6000);
    setProperty(&board[24], "Katugastota", 6000, 2750, 500, 1500, 6000);
   setSquare(&board[25], "Galle Railway Station", RAILWAY);
    board[25].price = 2000;
    board[25].mortgageValue = 1000;
    setProperty(&board[26], "Galle Fort", 6500, 3250, 600, 2000, 8000);
    setProperty(&board[27], "Unawatuna", 6800, 3250, 620, 2000, 8000);
    setSquare(&board[28], "National Water Supply and Drainage Board", UTILITY);
    board[28].price = 1500;
    board[28].mortgageValue = 750;
    setProperty(&board[29], "Hikkaduwa", 7000, 3250, 650, 2000, 8000);
    setSquare(&board[30], "Go To Jail", SPECIAL);
    setProperty(&board[31], "Jaffna Town", 8000, 4000, 750, 2500, 10000);
    setProperty(&board[32], "Nallur", 8300, 4000, 780, 2500, 10000);
    setSquare(&board[33], "Ceylinco Insurance", INSURANCE);
    setProperty(&board[34], "Trincomalee", 8500, 4000, 800, 2500, 10000);
    setSquare(&board[35], "Jaffna Railway Station", RAILWAY);
    board[35].price = 2000;
    board[35].mortgageValue = 1000;
    setSquare(&board[36], "National Event Card", EVENT);
    setProperty(&board[37], "Nuwara Eliya", 10000, 5000, 1000, 3000, 12000);
    setSquare(&board[38], "Bank of Ceylon", BANK);
    setProperty(&board[39], "Galle Face", 12000, 5000, 1200, 3000, 12000);
}