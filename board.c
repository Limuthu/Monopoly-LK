#include "types.h"
#include <stdio.h>
#include <string.h>

Square board[TOTAL_SQUARES];

void init_board(void) {
    // -------------------------------------------------------------
    // Index 0: GO
    // -------------------------------------------------------------
    board[0].index = 0;
    board[0].type = SQUARE_START;
    strcpy(board[0].name, "GO");

    // -------------------------------------------------------------
    // Index 1: Pettah
    // -------------------------------------------------------------
    board[1].index = 1;
    board[1].type = SQUARE_PROPERTY;
    strcpy(board[1].name, "Pettah");
    board[1].data.property.group = GROUP_BROWN;
    board[1].data.property.price = 2000;
    board[1].data.property.mortgage_value = 500;
    board[1].data.property.house_cost = 750;
    board[1].data.property.hotel_cost = 1250;
    board[1].data.property.owner_id = -1;
    board[1].data.property.num_houses = 0;
    board[1].data.property.has_hotel = 0;
    board[1].data.property.is_mortgaged = 0;
    board[1].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 2: Community Development Fund
    // -------------------------------------------------------------
    board[2].index = 2;
    board[2].type = SQUARE_EVENT;
    strcpy(board[2].name, "Community Development Fund");

    // -------------------------------------------------------------
    // Index 3: Maradana
    // -------------------------------------------------------------
    board[3].index = 3;
    board[3].type = SQUARE_PROPERTY;
    strcpy(board[3].name, "Maradana");
    board[3].data.property.group = GROUP_BROWN;
    board[3].data.property.price = 2000;
    board[3].data.property.mortgage_value = 500;
    board[3].data.property.house_cost = 750;
    board[3].data.property.hotel_cost = 1250;
    board[3].data.property.owner_id = -1;
    board[3].data.property.num_houses = 0;
    board[3].data.property.has_hotel = 0;
    board[3].data.property.is_mortgaged = 0;
    board[3].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 4: Income Tax
    // -------------------------------------------------------------
    board[4].index = 4;
    board[4].type = SQUARE_TAX;
    strcpy(board[4].name, "Income Tax");

    // -------------------------------------------------------------
    // Index 5: Colombo Fort Railway Station
    // -------------------------------------------------------------
    board[5].index = 5;
    board[5].type = SQUARE_RAILWAY;
    strcpy(board[5].name, "Colombo Fort Railway Station");
    board[5].data.railway.price = 2000;
    board[5].data.railway.owner_id = -1;
    board[5].data.railway.is_mortgaged = 0;

    // -------------------------------------------------------------
    // Index 6: Bambalapitiya
    // -------------------------------------------------------------
    board[6].index = 6;
    board[6].type = SQUARE_PROPERTY;
    strcpy(board[6].name, "Bambalapitiya");
    board[6].data.property.group = GROUP_LIGHT_BLUE;
    board[6].data.property.price = 3000;
    board[6].data.property.mortgage_value = 1000;
    board[6].data.property.house_cost = 1750;
    board[6].data.property.hotel_cost = 2250;
    board[6].data.property.owner_id = -1;
    board[6].data.property.num_houses = 0;
    board[6].data.property.has_hotel = 0;
    board[6].data.property.is_mortgaged = 0;
    board[6].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 7: National Event Card
    // -------------------------------------------------------------
    board[7].index = 7;
    board[7].type = SQUARE_EVENT;
    strcpy(board[7].name, "National Event Card");

    // -------------------------------------------------------------
    // Index 8: Wellawatte
    // -------------------------------------------------------------
    board[8].index = 8;
    board[8].type = SQUARE_PROPERTY;
    strcpy(board[8].name, "Wellawatte");
    board[8].data.property.group = GROUP_LIGHT_BLUE;
    board[8].data.property.price = 3000;
    board[8].data.property.mortgage_value = 1000;
    board[8].data.property.house_cost = 1750;
    board[8].data.property.hotel_cost = 2250;
    board[8].data.property.owner_id = -1;
    board[8].data.property.num_houses = 0;
    board[8].data.property.has_hotel = 0;
    board[8].data.property.is_mortgaged = 0;
    board[8].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 9: Mount Lavinia
    // -------------------------------------------------------------
    board[9].index = 9;
    board[9].type = SQUARE_PROPERTY;
    strcpy(board[9].name, "Mount Lavinia");
    board[9].data.property.group = GROUP_LIGHT_BLUE;
    board[9].data.property.price = 3000;
    board[9].data.property.mortgage_value = 1000;
    board[9].data.property.house_cost = 1750;
    board[9].data.property.hotel_cost = 2250;
    board[9].data.property.owner_id = -1;
    board[9].data.property.num_houses = 0;
    board[9].data.property.has_hotel = 0;
    board[9].data.property.is_mortgaged = 0;
    board[9].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 10: Jail Just Visiting
    // -------------------------------------------------------------
    board[10].index = 10;
    board[10].type = SQUARE_SPECIAL;
    strcpy(board[10].name, "Jail Just Visiting");

    // -------------------------------------------------------------
    // Index 11: Nugegoda
    // -------------------------------------------------------------
    board[11].index = 11;
    board[11].type = SQUARE_PROPERTY;
    strcpy(board[11].name, "Nugegoda");
    board[11].data.property.group = GROUP_PINK;
    board[11].data.property.price = 4000;
    board[11].data.property.mortgage_value = 1250;
    board[11].data.property.house_cost = 2500;
    board[11].data.property.hotel_cost = 3500;
    board[11].data.property.owner_id = -1;
    board[11].data.property.num_houses = 0;
    board[11].data.property.has_hotel = 0;
    board[11].data.property.is_mortgaged = 0;
    board[11].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 12: Ceylon Electricity Board
    // -------------------------------------------------------------
    board[12].index = 12;
    board[12].type = SQUARE_UTILITY;
    strcpy(board[12].name, "Ceylon Electricity Board");
    board[12].data.utility.price = 1500;
    board[12].data.utility.owner_id = -1;
    board[12].data.utility.is_mortgaged = 0;

    // -------------------------------------------------------------
    // Index 13: Maharagama
    // -------------------------------------------------------------
    board[13].index = 13;
    board[13].type = SQUARE_PROPERTY;
    strcpy(board[13].name, "Maharagama");
    board[13].data.property.group = GROUP_PINK;
    board[13].data.property.price = 4000;
    board[13].data.property.mortgage_value = 1250;
    board[13].data.property.house_cost = 2500;
    board[13].data.property.hotel_cost = 3500;
    board[13].data.property.owner_id = -1;
    board[13].data.property.num_houses = 0;
    board[13].data.property.has_hotel = 0;
    board[13].data.property.is_mortgaged = 0;
    board[13].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 14: Kottawa
    // -------------------------------------------------------------
    board[14].index = 14;
    board[14].type = SQUARE_PROPERTY;
    strcpy(board[14].name, "Kottawa");
    board[14].data.property.group = GROUP_PINK;
    board[14].data.property.price = 4000;
    board[14].data.property.mortgage_value = 1250;
    board[14].data.property.house_cost = 2500;
    board[14].data.property.hotel_cost = 3500;
    board[14].data.property.owner_id = -1;
    board[14].data.property.num_houses = 0;
    board[14].data.property.has_hotel = 0;
    board[14].data.property.is_mortgaged = 0;
    board[14].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 15: Kandy Railway Station
    // -------------------------------------------------------------
    board[15].index = 15;
    board[15].type = SQUARE_RAILWAY;
    strcpy(board[15].name, "Kandy Railway Station");
    board[15].data.railway.price = 2000;
    board[15].data.railway.owner_id = -1;
    board[15].data.railway.is_mortgaged = 0;

    // -------------------------------------------------------------
    // Index 16: Negombo
    // -------------------------------------------------------------
    board[16].index = 16;
    board[16].type = SQUARE_PROPERTY;
    strcpy(board[16].name, "Negombo");
    board[16].data.property.group = GROUP_ORANGE;
    board[16].data.property.price = 5000;
    board[16].data.property.mortgage_value = 1500;
    board[16].data.property.house_cost = 3500;
    board[16].data.property.hotel_cost = 4500;
    board[16].data.property.owner_id = -1;
    board[16].data.property.num_houses = 0;
    board[16].data.property.has_hotel = 0;
    board[16].data.property.is_mortgaged = 0;
    board[16].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 17: Sri Lanka Insurance
    // -------------------------------------------------------------
    board[17].index = 17;
    board[17].type = SQUARE_INSURANCE;
    strcpy(board[17].name, "Sri Lanka Insurance");

    // -------------------------------------------------------------
    // Index 18: Katunayake
    // -------------------------------------------------------------
    board[18].index = 18;
    board[18].type = SQUARE_PROPERTY;
    strcpy(board[18].name, "Katunayake");
    board[18].data.property.group = GROUP_ORANGE;
    board[18].data.property.price = 5000;
    board[18].data.property.mortgage_value = 1500;
    board[18].data.property.house_cost = 3500;
    board[18].data.property.hotel_cost = 4500;
    board[18].data.property.owner_id = -1;
    board[18].data.property.num_houses = 0;
    board[18].data.property.has_hotel = 0;
    board[18].data.property.is_mortgaged = 0;
    board[18].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 19: Ja-Ela
    // -------------------------------------------------------------
    board[19].index = 19;
    board[19].type = SQUARE_PROPERTY;
    strcpy(board[19].name, "Ja-Ela");
    board[19].data.property.group = GROUP_ORANGE;
    board[19].data.property.price = 5000;
    board[19].data.property.mortgage_value = 1500;
    board[19].data.property.house_cost = 3500;
    board[19].data.property.hotel_cost = 4500;
    board[19].data.property.owner_id = -1;
    board[19].data.property.num_houses = 0;
    board[19].data.property.has_hotel = 0;
    board[19].data.property.is_mortgaged = 0;
    board[19].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 20: Free Parking
    // -------------------------------------------------------------
    board[20].index = 20;
    board[20].type = SQUARE_SPECIAL;
    strcpy(board[20].name, "Free Parking");

    // -------------------------------------------------------------
    // Index 21: Kandy City
    // -------------------------------------------------------------
    board[21].index = 21;
    board[21].type = SQUARE_PROPERTY;
    strcpy(board[21].name, "Kandy City");
    board[21].data.property.group = GROUP_RED;
    board[21].data.property.price = 5500;
    board[21].data.property.mortgage_value = 1500;
    board[21].data.property.house_cost = 2750;
    board[21].data.property.hotel_cost = 6000;
    board[21].data.property.owner_id = -1;
    board[21].data.property.num_houses = 0;
    board[21].data.property.has_hotel = 0;
    board[21].data.property.is_mortgaged = 0;
    board[21].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 22: National Event Card
    // -------------------------------------------------------------
    board[22].index = 22;
    board[22].type = SQUARE_EVENT;
    strcpy(board[22].name, "National Event Card");

    // -------------------------------------------------------------
    // Index 23: Peradeniya
    // -------------------------------------------------------------
    board[23].index = 23;
    board[23].type = SQUARE_PROPERTY;
    strcpy(board[23].name, "Peradeniya");
    board[23].data.property.group = GROUP_RED;
    board[23].data.property.price = 5500;
    board[23].data.property.mortgage_value = 1500;
    board[23].data.property.house_cost = 2750;
    board[23].data.property.hotel_cost = 6000;
    board[23].data.property.owner_id = -1;
    board[23].data.property.num_houses = 0;
    board[23].data.property.has_hotel = 0;
    board[23].data.property.is_mortgaged = 0;
    board[23].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 24: Katugastota
    // -------------------------------------------------------------
    board[24].index = 24;
    board[24].type = SQUARE_PROPERTY;
    strcpy(board[24].name, "Katugastota");
    board[24].data.property.group = GROUP_RED;
    board[24].data.property.price = 5500;
    board[24].data.property.mortgage_value = 1500;
    board[24].data.property.house_cost = 2750;
    board[24].data.property.hotel_cost = 6000;
    board[24].data.property.owner_id = -1;
    board[24].data.property.num_houses = 0;
    board[24].data.property.has_hotel = 0;
    board[24].data.property.is_mortgaged = 0;
    board[24].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 25: Galle Railway Station
    // -------------------------------------------------------------
    board[25].index = 25;
    board[25].type = SQUARE_RAILWAY;
    strcpy(board[25].name, "Galle Railway Station");
    board[25].data.railway.price = 2000;
    board[25].data.railway.owner_id = -1;
    board[25].data.railway.is_mortgaged = 0;

    // -------------------------------------------------------------
    // Index 26: Galle Fort
    // -------------------------------------------------------------
    board[26].index = 26;
    board[26].type = SQUARE_PROPERTY;
    strcpy(board[26].name, "Galle Fort");
    board[26].data.property.group = GROUP_YELLOW;
    board[26].data.property.price = 6500;
    board[26].data.property.mortgage_value = 2000;
    board[26].data.property.house_cost = 3250;
    board[26].data.property.hotel_cost = 8000;
    board[26].data.property.owner_id = -1;
    board[26].data.property.num_houses = 0;
    board[26].data.property.has_hotel = 0;
    board[26].data.property.is_mortgaged = 0;
    board[26].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 27: Unawatuna
    // -------------------------------------------------------------
    board[27].index = 27;
    board[27].type = SQUARE_PROPERTY;
    strcpy(board[27].name, "Unawatuna");
    board[27].data.property.group = GROUP_YELLOW;
    board[27].data.property.price = 6500;
    board[27].data.property.mortgage_value = 2000;
    board[27].data.property.house_cost = 3250;
    board[27].data.property.hotel_cost = 8000;
    board[27].data.property.owner_id = -1;
    board[27].data.property.num_houses = 0;
    board[27].data.property.has_hotel = 0;
    board[27].data.property.is_mortgaged = 0;
    board[27].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 28: National Water Supply and Drainage Board
    // -------------------------------------------------------------
    board[28].index = 28;
    board[28].type = SQUARE_UTILITY;
    strcpy(board[28].name, "National Water Supply and Drainage Board");
    board[28].data.utility.price = 1500;
    board[28].data.utility.owner_id = -1;
    board[28].data.utility.is_mortgaged = 0;

    // -------------------------------------------------------------
    // Index 29: Hikkaduwa
    // -------------------------------------------------------------
    board[29].index = 29;
    board[29].type = SQUARE_PROPERTY;
    strcpy(board[29].name, "Hikkaduwa");
    board[29].data.property.group = GROUP_YELLOW;
    board[29].data.property.price = 6500;
    board[29].data.property.mortgage_value = 2000;
    board[29].data.property.house_cost = 3250;
    board[29].data.property.hotel_cost = 8000;
    board[29].data.property.owner_id = -1;
    board[29].data.property.num_houses = 0;
    board[29].data.property.has_hotel = 0;
    board[29].data.property.is_mortgaged = 0;
    board[29].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 30: Go To Jail
    // -------------------------------------------------------------
    board[30].index = 30;
    board[30].type = SQUARE_SPECIAL;
    strcpy(board[30].name, "Go To Jail");

    // -------------------------------------------------------------
    // Index 31: Jaffna Town
    // -------------------------------------------------------------
    board[31].index = 31;
    board[31].type = SQUARE_PROPERTY;
    strcpy(board[31].name, "Jaffna Town");
    board[31].data.property.group = GROUP_GREEN;
    board[31].data.property.price = 8000;
    board[31].data.property.mortgage_value = 2500;
    board[31].data.property.house_cost = 4000;
    board[31].data.property.hotel_cost = 10000;
    board[31].data.property.owner_id = -1;
    board[31].data.property.num_houses = 0;
    board[31].data.property.has_hotel = 0;
    board[31].data.property.is_mortgaged = 0;
    board[31].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 32: Nallur
    // -------------------------------------------------------------
    board[32].index = 32;
    board[32].type = SQUARE_PROPERTY;
    strcpy(board[32].name, "Nallur");
    board[32].data.property.group = GROUP_GREEN;
    board[32].data.property.price = 8000;
    board[32].data.property.mortgage_value = 2500;
    board[32].data.property.house_cost = 4000;
    board[32].data.property.hotel_cost = 10000;
    board[32].data.property.owner_id = -1;
    board[32].data.property.num_houses = 0;
    board[32].data.property.has_hotel = 0;
    board[32].data.property.is_mortgaged = 0;
    board[32].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 33: Ceylinco Insurance
    // -------------------------------------------------------------
    board[33].index = 33;
    board[33].type = SQUARE_INSURANCE;
    strcpy(board[33].name, "Ceylinco Insurance");

    // -------------------------------------------------------------
    // Index 34: Trincomalee
    // -------------------------------------------------------------
    board[34].index = 34;
    board[34].type = SQUARE_PROPERTY;
    strcpy(board[34].name, "Trincomalee");
    board[34].data.property.group = GROUP_GREEN;
    board[34].data.property.price = 8000;
    board[34].data.property.mortgage_value = 2500;
    board[34].data.property.house_cost = 4000;
    board[34].data.property.hotel_cost = 10000;
    board[34].data.property.owner_id = -1;
    board[34].data.property.num_houses = 0;
    board[34].data.property.has_hotel = 0;
    board[34].data.property.is_mortgaged = 0;
    board[34].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 35: Jaffna Railway Station
    // -------------------------------------------------------------
    board[35].index = 35;
    board[35].type = SQUARE_RAILWAY;
    strcpy(board[35].name, "Jaffna Railway Station");
    board[35].data.railway.price = 2000;
    board[35].data.railway.owner_id = -1;
    board[35].data.railway.is_mortgaged = 0;

    // -------------------------------------------------------------
    // Index 36: National Event Card
    // -------------------------------------------------------------
    board[36].index = 36;
    board[36].type = SQUARE_EVENT;
    strcpy(board[36].name, "National Event Card");

    // -------------------------------------------------------------
    // Index 37: Nuwara Eliya
    // -------------------------------------------------------------
    board[37].index = 37;
    board[37].type = SQUARE_PROPERTY;
    strcpy(board[37].name, "Nuwara Eliya");
    board[37].data.property.group = GROUP_DARK_BLUE;
    board[37].data.property.price = 10000;
    board[37].data.property.mortgage_value = 3000;
    board[37].data.property.house_cost = 5000;
    board[37].data.property.hotel_cost = 12000;
    board[37].data.property.owner_id = -1;
    board[37].data.property.num_houses = 0;
    board[37].data.property.has_hotel = 0;
    board[37].data.property.is_mortgaged = 0;
    board[37].data.property.is_insured = 0;

    // -------------------------------------------------------------
    // Index 38: Bank of Ceylon
    // -------------------------------------------------------------
    board[38].index = 38;
    board[38].type = SQUARE_BANK;
    strcpy(board[38].name, "Bank of Ceylon");

    // -------------------------------------------------------------
    // Index 39: Galle Face
    // -------------------------------------------------------------
    board[39].index = 39;
    board[39].type = SQUARE_PROPERTY;
    strcpy(board[39].name, "Galle Face");
    board[39].data.property.group = GROUP_DARK_BLUE;
    board[39].data.property.price = 10000;
    board[39].data.property.mortgage_value = 3000;
    board[39].data.property.house_cost = 5000;
    board[39].data.property.hotel_cost = 12000;
    board[39].data.property.owner_id = -1;
    board[39].data.property.num_houses = 0;
    board[39].data.property.has_hotel = 0;
    board[39].data.property.is_mortgaged = 0;
    board[39].data.property.is_insured = 0;
}
