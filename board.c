#include "types.h"
#include <stdio.h>
#include <string.h>

void init_board(GameState *game) {

  for (int i = 0; i < 40; i++) {
    game->board[i].owner_id = -1;
  }

  // -------------------------------------------------------------
  // Index 0: GO
  // -------------------------------------------------------------
  game->board[0].index = 0;
  game->board[0].type = SQUARE_START;
  strcpy(game->board[0].name, "GO");

  // -------------------------------------------------------------
  // Index 1: Pettah
  // -------------------------------------------------------------
  game->board[1].index = 1;
  game->board[1].type = SQUARE_PROPERTY;
  strcpy(game->board[1].name, "Pettah");
  game->board[1].data.property.group = GROUP_BROWN;
  game->board[1].data.property.price = 1500;
  game->board[1].data.property.base_rent = 100;
  game->board[1].data.property.mortgage_value = 750;
  game->board[1].data.property.house_cost = 500;
  game->board[1].data.property.hotel_cost = 2000;
  game->board[1].data.property.owner_id = -1;
  game->board[1].data.property.num_houses = 0;
  game->board[1].data.property.has_hotel = 0;
  game->board[1].data.property.is_mortgaged = 0;
  game->board[1].data.property.is_insured = 0;
  game->board[1].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 2: Community Development Fund
  // -------------------------------------------------------------
  game->board[2].index = 2;
  game->board[2].type = SQUARE_EVENT;
  strcpy(game->board[2].name, "Community Development Fund");

  // -------------------------------------------------------------
  // Index 3: Maradana
  // -------------------------------------------------------------
  game->board[3].index = 3;
  game->board[3].type = SQUARE_PROPERTY;
  strcpy(game->board[3].name, "Maradana");
  game->board[3].data.property.group = GROUP_BROWN;
  game->board[3].data.property.price = 1800;
  game->board[3].data.property.base_rent = 120;
  game->board[3].data.property.mortgage_value = 750;
  game->board[3].data.property.house_cost = 500;
  game->board[3].data.property.hotel_cost = 2000;
  game->board[3].data.property.owner_id = -1;
  game->board[3].data.property.num_houses = 0;
  game->board[3].data.property.has_hotel = 0;
  game->board[3].data.property.is_mortgaged = 0;
  game->board[3].data.property.is_insured = 0;
  game->board[3].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 4: Income Tax
  // -------------------------------------------------------------
  game->board[4].index = 4;
  game->board[4].type = SQUARE_TAX;
  strcpy(game->board[4].name, "Income Tax");

  // -------------------------------------------------------------
  // Index 5: Colombo Fort Railway Station
  // -------------------------------------------------------------
  game->board[5].index = 5;
  game->board[5].type = SQUARE_RAILWAY;
  strcpy(game->board[5].name, "Colombo Fort Railway Station");
  game->board[5].data.railway.price = 2000;
  game->board[5].data.railway.mortgage_value = 1000;
  game->board[5].data.railway.owner_id = -1;
  game->board[5].data.railway.is_mortgaged = 0;
  game->board[5].data.railway.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 6: Bambalapitiya
  // -------------------------------------------------------------
  game->board[6].index = 6;
  game->board[6].type = SQUARE_PROPERTY;
  strcpy(game->board[6].name, "Bambalapitiya");
  game->board[6].data.property.group = GROUP_LIGHT_BLUE;
  game->board[6].data.property.price = 2500;
  game->board[6].data.property.base_rent = 180;
  game->board[6].data.property.mortgage_value = 1250;
  game->board[6].data.property.house_cost = 750;
  game->board[6].data.property.hotel_cost = 3000;
  game->board[6].data.property.owner_id = -1;
  game->board[6].data.property.num_houses = 0;
  game->board[6].data.property.has_hotel = 0;
  game->board[6].data.property.is_mortgaged = 0;
  game->board[6].data.property.is_insured = 0;
  game->board[6].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 7: National Event Card
  // -------------------------------------------------------------
  game->board[7].index = 7;
  game->board[7].type = SQUARE_EVENT;
  strcpy(game->board[7].name, "National Event Card");

  // -------------------------------------------------------------
  // Index 8: Wellawatte
  // -------------------------------------------------------------
  game->board[8].index = 8;
  game->board[8].type = SQUARE_PROPERTY;
  strcpy(game->board[8].name, "Wellawatte");
  game->board[8].data.property.group = GROUP_LIGHT_BLUE;
  game->board[8].data.property.price = 2700;
  game->board[8].data.property.base_rent = 200;
  game->board[8].data.property.mortgage_value = 1250;
  game->board[8].data.property.house_cost = 750;
  game->board[8].data.property.hotel_cost = 3000;
  game->board[8].data.property.owner_id = -1;
  game->board[8].data.property.num_houses = 0;
  game->board[8].data.property.has_hotel = 0;
  game->board[8].data.property.is_mortgaged = 0;
  game->board[8].data.property.is_insured = 0;
  game->board[8].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 9: Mount Lavinia
  // -------------------------------------------------------------
  game->board[9].index = 9;
  game->board[9].type = SQUARE_PROPERTY;
  strcpy(game->board[9].name, "Mount Lavinia");
  game->board[9].data.property.group = GROUP_LIGHT_BLUE;
  game->board[9].data.property.price = 3000;
  game->board[9].data.property.base_rent = 220;
  game->board[9].data.property.mortgage_value = 1250;
  game->board[9].data.property.house_cost = 750;
  game->board[9].data.property.hotel_cost = 3000;
  game->board[9].data.property.owner_id = -1;
  game->board[9].data.property.num_houses = 0;
  game->board[9].data.property.has_hotel = 0;
  game->board[9].data.property.is_mortgaged = 0;
  game->board[9].data.property.is_insured = 0;
  game->board[9].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 10: Jail Just Visiting
  // -------------------------------------------------------------
  game->board[10].index = 10;
  game->board[10].type = SQUARE_SPECIAL;
  strcpy(game->board[10].name, "Jail / Just Visiting");

  // -------------------------------------------------------------
  // Index 11: Nugegoda
  // -------------------------------------------------------------
  game->board[11].index = 11;
  game->board[11].type = SQUARE_PROPERTY;
  strcpy(game->board[11].name, "Nugegoda");
  game->board[11].data.property.group = GROUP_PINK;
  game->board[11].data.property.price = 3500;
  game->board[11].data.property.base_rent = 260;
  game->board[11].data.property.mortgage_value = 1750;
  game->board[11].data.property.house_cost = 1000;
  game->board[11].data.property.hotel_cost = 4000;
  game->board[11].data.property.owner_id = -1;
  game->board[11].data.property.num_houses = 0;
  game->board[11].data.property.has_hotel = 0;
  game->board[11].data.property.is_mortgaged = 0;
  game->board[11].data.property.is_insured = 0;
  game->board[11].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 12: Ceylon Electricity Board
  // -------------------------------------------------------------
  game->board[12].index = 12;
  game->board[12].type = SQUARE_UTILITY;
  strcpy(game->board[12].name, "Ceylon Electricity Board");
  game->board[12].data.utility.price = 1500;
  game->board[12].data.utility.mortgage_value = 750;
  game->board[12].data.utility.owner_id = -1;
  game->board[12].data.utility.is_mortgaged = 0;
  game->board[12].data.utility.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 13: Maharagama
  // -------------------------------------------------------------
  game->board[13].index = 13;
  game->board[13].type = SQUARE_PROPERTY;
  strcpy(game->board[13].name, "Maharagama");
  game->board[13].data.property.group = GROUP_PINK;
  game->board[13].data.property.price = 3800;
  game->board[13].data.property.base_rent = 280;
  game->board[13].data.property.mortgage_value = 1750;
  game->board[13].data.property.house_cost = 1000;
  game->board[13].data.property.hotel_cost = 4000;
  game->board[13].data.property.owner_id = -1;
  game->board[13].data.property.num_houses = 0;
  game->board[13].data.property.has_hotel = 0;
  game->board[13].data.property.is_mortgaged = 0;
  game->board[13].data.property.is_insured = 0;
  game->board[13].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 14: Kottawa
  // -------------------------------------------------------------
  game->board[14].index = 14;
  game->board[14].type = SQUARE_PROPERTY;
  strcpy(game->board[14].name, "Kottawa");
  game->board[14].data.property.group = GROUP_PINK;
  game->board[14].data.property.price = 4000;
  game->board[14].data.property.base_rent = 300;
  game->board[14].data.property.mortgage_value = 1750;
  game->board[14].data.property.house_cost = 1000;
  game->board[14].data.property.hotel_cost = 4000;
  game->board[14].data.property.owner_id = -1;
  game->board[14].data.property.num_houses = 0;
  game->board[14].data.property.has_hotel = 0;
  game->board[14].data.property.is_mortgaged = 0;
  game->board[14].data.property.is_insured = 0;
  game->board[14].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 15: Kandy Railway Station
  // -------------------------------------------------------------
  game->board[15].index = 15;
  game->board[15].type = SQUARE_RAILWAY;
  strcpy(game->board[15].name, "Kandy Railway Station");
  game->board[15].data.railway.price = 2000;
  game->board[15].data.railway.mortgage_value = 1000;
  game->board[15].data.railway.owner_id = -1;
  game->board[15].data.railway.is_mortgaged = 0;
  game->board[15].data.railway.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 16: Negombo
  // -------------------------------------------------------------
  game->board[16].index = 16;
  game->board[16].type = SQUARE_PROPERTY;
  strcpy(game->board[16].name, "Negombo");
  game->board[16].data.property.group = GROUP_ORANGE;
  game->board[16].data.property.price = 4500;
  game->board[16].data.property.base_rent = 350;
  game->board[16].data.property.mortgage_value = 2250;
  game->board[16].data.property.house_cost = 1250;
  game->board[16].data.property.hotel_cost = 5000;
  game->board[16].data.property.owner_id = -1;
  game->board[16].data.property.num_houses = 0;
  game->board[16].data.property.has_hotel = 0;
  game->board[16].data.property.is_mortgaged = 0;
  game->board[16].data.property.is_insured = 0;
  game->board[16].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 17: Sri Lanka Insurance
  // -------------------------------------------------------------
  game->board[17].index = 17;
  game->board[17].type = SQUARE_INSURANCE;
  strcpy(game->board[17].name, "Sri Lanka Insurance");

  // -------------------------------------------------------------
  // Index 18: Katunayake
  // -------------------------------------------------------------
  game->board[18].index = 18;
  game->board[18].type = SQUARE_PROPERTY;
  strcpy(game->board[18].name, "Katunayake");
  game->board[18].data.property.group = GROUP_ORANGE;
  game->board[18].data.property.price = 4700;
  game->board[18].data.property.base_rent = 370;
  game->board[18].data.property.mortgage_value = 2250;
  game->board[18].data.property.house_cost = 1250;
  game->board[18].data.property.hotel_cost = 5000;
  game->board[18].data.property.owner_id = -1;
  game->board[18].data.property.num_houses = 0;
  game->board[18].data.property.has_hotel = 0;
  game->board[18].data.property.is_mortgaged = 0;
  game->board[18].data.property.is_insured = 0;
  game->board[18].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 19: Ja-Ela
  // -------------------------------------------------------------
  game->board[19].index = 19;
  game->board[19].type = SQUARE_PROPERTY;
  strcpy(game->board[19].name, "Ja-Ela");
  game->board[19].data.property.group = GROUP_ORANGE;
  game->board[19].data.property.price = 5000;
  game->board[19].data.property.base_rent = 400;
  game->board[19].data.property.mortgage_value = 2250;
  game->board[19].data.property.house_cost = 1250;
  game->board[19].data.property.hotel_cost = 5000;
  game->board[19].data.property.owner_id = -1;
  game->board[19].data.property.num_houses = 0;
  game->board[19].data.property.has_hotel = 0;
  game->board[19].data.property.is_mortgaged = 0;
  game->board[19].data.property.is_insured = 0;
  game->board[19].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 20: Free Parking
  // -------------------------------------------------------------
  game->board[20].index = 20;
  game->board[20].type = SQUARE_SPECIAL;
  strcpy(game->board[20].name, "Free Parking");

  // -------------------------------------------------------------
  // Index 21: Kandy City
  // -------------------------------------------------------------
  game->board[21].index = 21;
  game->board[21].type = SQUARE_PROPERTY;
  strcpy(game->board[21].name, "Kandy City");
  game->board[21].data.property.group = GROUP_RED;
  game->board[21].data.property.price = 5500;
  game->board[21].data.property.base_rent = 450;
  game->board[21].data.property.mortgage_value = 2750;
  game->board[21].data.property.house_cost = 1500;
  game->board[21].data.property.hotel_cost = 6000;
  game->board[21].data.property.owner_id = -1;
  game->board[21].data.property.num_houses = 0;
  game->board[21].data.property.has_hotel = 0;
  game->board[21].data.property.is_mortgaged = 0;
  game->board[21].data.property.is_insured = 0;
  game->board[21].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 22: National Event Card
  // -------------------------------------------------------------
  game->board[22].index = 22;
  game->board[22].type = SQUARE_EVENT;
  strcpy(game->board[22].name, "National Event Card");

  // -------------------------------------------------------------
  // Index 23: Peradeniya
  // -------------------------------------------------------------
  game->board[23].index = 23;
  game->board[23].type = SQUARE_PROPERTY;
  strcpy(game->board[23].name, "Peradeniya");
  game->board[23].data.property.group = GROUP_RED;
  game->board[23].data.property.price = 5800;
  game->board[23].data.property.base_rent = 480;
  game->board[23].data.property.mortgage_value = 2750;
  game->board[23].data.property.house_cost = 1500;
  game->board[23].data.property.hotel_cost = 6000;
  game->board[23].data.property.owner_id = -1;
  game->board[23].data.property.num_houses = 0;
  game->board[23].data.property.has_hotel = 0;
  game->board[23].data.property.is_mortgaged = 0;
  game->board[23].data.property.is_insured = 0;
  game->board[23].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 24: Katugastota
  // -------------------------------------------------------------
  game->board[24].index = 24;
  game->board[24].type = SQUARE_PROPERTY;
  strcpy(game->board[24].name, "Katugastota");
  game->board[24].data.property.group = GROUP_RED;
  game->board[24].data.property.price = 6000;
  game->board[24].data.property.base_rent = 500;
  game->board[24].data.property.mortgage_value = 2750;
  game->board[24].data.property.house_cost = 1500;
  game->board[24].data.property.hotel_cost = 6000;
  game->board[24].data.property.owner_id = -1;
  game->board[24].data.property.num_houses = 0;
  game->board[24].data.property.has_hotel = 0;
  game->board[24].data.property.is_mortgaged = 0;
  game->board[24].data.property.is_insured = 0;
  game->board[24].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 25: Galle Railway Station
  // -------------------------------------------------------------
  game->board[25].index = 25;
  game->board[25].type = SQUARE_RAILWAY;
  strcpy(game->board[25].name, "Galle Railway Station");
  game->board[25].data.railway.price = 2000;
  game->board[25].data.railway.mortgage_value = 1000;
  game->board[25].data.railway.owner_id = -1;
  game->board[25].data.railway.is_mortgaged = 0;
  game->board[25].data.railway.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 26: Galle Fort
  // -------------------------------------------------------------
  game->board[26].index = 26;
  game->board[26].type = SQUARE_PROPERTY;
  strcpy(game->board[26].name, "Galle Fort");
  game->board[26].data.property.group = GROUP_YELLOW;
  game->board[26].data.property.price = 6500;
  game->board[26].data.property.base_rent = 600;
  game->board[26].data.property.mortgage_value = 3250;
  game->board[26].data.property.house_cost = 2000;
  game->board[26].data.property.hotel_cost = 8000;
  game->board[26].data.property.owner_id = -1;
  game->board[26].data.property.num_houses = 0;
  game->board[26].data.property.has_hotel = 0;
  game->board[26].data.property.is_mortgaged = 0;
  game->board[26].data.property.is_insured = 0;
  game->board[26].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 27: Unawatuna
  // -------------------------------------------------------------
  game->board[27].index = 27;
  game->board[27].type = SQUARE_PROPERTY;
  strcpy(game->board[27].name, "Unawatuna");
  game->board[27].data.property.group = GROUP_YELLOW;
  game->board[27].data.property.price = 6800;
  game->board[27].data.property.base_rent = 620;
  game->board[27].data.property.mortgage_value = 3250;
  game->board[27].data.property.house_cost = 2000;
  game->board[27].data.property.hotel_cost = 8000;
  game->board[27].data.property.owner_id = -1;
  game->board[27].data.property.num_houses = 0;
  game->board[27].data.property.has_hotel = 0;
  game->board[27].data.property.is_mortgaged = 0;
  game->board[27].data.property.is_insured = 0;
  game->board[27].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 28: National Water Supply and Drainage Board
  // -------------------------------------------------------------
  game->board[28].index = 28;
  game->board[28].type = SQUARE_UTILITY;
  strcpy(game->board[28].name, "National Water Supply and Drainage Board");
  game->board[28].data.utility.price = 1500;
  game->board[28].data.utility.mortgage_value = 750;
  game->board[28].data.utility.owner_id = -1;
  game->board[28].data.utility.is_mortgaged = 0;
  game->board[28].data.utility.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 29: Hikkaduwa
  // -------------------------------------------------------------
  game->board[29].index = 29;
  game->board[29].type = SQUARE_PROPERTY;
  strcpy(game->board[29].name, "Hikkaduwa");
  game->board[29].data.property.group = GROUP_YELLOW;
  game->board[29].data.property.price = 7000;
  game->board[29].data.property.base_rent = 650;
  game->board[29].data.property.mortgage_value = 3250;
  game->board[29].data.property.house_cost = 2000;
  game->board[29].data.property.hotel_cost = 8000;
  game->board[29].data.property.owner_id = -1;
  game->board[29].data.property.num_houses = 0;
  game->board[29].data.property.has_hotel = 0;
  game->board[29].data.property.is_mortgaged = 0;
  game->board[29].data.property.is_insured = 0;
  game->board[29].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 30: Go To Jail
  // -------------------------------------------------------------
  game->board[30].index = 30;
  game->board[30].type = SQUARE_SPECIAL;
  strcpy(game->board[30].name, "Go To Jail");

  // -------------------------------------------------------------
  // Index 31: Jaffna Town
  // -------------------------------------------------------------
  game->board[31].index = 31;
  game->board[31].type = SQUARE_PROPERTY;
  strcpy(game->board[31].name, "Jaffna Town");
  game->board[31].data.property.group = GROUP_GREEN;
  game->board[31].data.property.price = 8000;
  game->board[31].data.property.base_rent = 750;
  game->board[31].data.property.mortgage_value = 4000;
  game->board[31].data.property.house_cost = 2500;
  game->board[31].data.property.hotel_cost = 10000;
  game->board[31].data.property.owner_id = -1;
  game->board[31].data.property.num_houses = 0;
  game->board[31].data.property.has_hotel = 0;
  game->board[31].data.property.is_mortgaged = 0;
  game->board[31].data.property.is_insured = 0;
  game->board[31].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 32: Nallur
  // -------------------------------------------------------------
  game->board[32].index = 32;
  game->board[32].type = SQUARE_PROPERTY;
  strcpy(game->board[32].name, "Nallur");
  game->board[32].data.property.group = GROUP_GREEN;
  game->board[32].data.property.price = 8300;
  game->board[32].data.property.base_rent = 780;
  game->board[32].data.property.mortgage_value = 4000;
  game->board[32].data.property.house_cost = 2500;
  game->board[32].data.property.hotel_cost = 10000;
  game->board[32].data.property.owner_id = -1;
  game->board[32].data.property.num_houses = 0;
  game->board[32].data.property.has_hotel = 0;
  game->board[32].data.property.is_mortgaged = 0;
  game->board[32].data.property.is_insured = 0;
  game->board[32].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 33: Ceylinco Insurance
  // -------------------------------------------------------------
  game->board[33].index = 33;
  game->board[33].type = SQUARE_INSURANCE;
  strcpy(game->board[33].name, "Ceylinco Insurance");

  // -------------------------------------------------------------
  // Index 34: Trincomalee
  // -------------------------------------------------------------
  game->board[34].index = 34;
  game->board[34].type = SQUARE_PROPERTY;
  strcpy(game->board[34].name, "Trincomalee");
  game->board[34].data.property.group = GROUP_GREEN;
  game->board[34].data.property.price = 8500;
  game->board[34].data.property.base_rent = 800;
  game->board[34].data.property.mortgage_value = 4000;
  game->board[34].data.property.house_cost = 2500;
  game->board[34].data.property.hotel_cost = 10000;
  game->board[34].data.property.owner_id = -1;
  game->board[34].data.property.num_houses = 0;
  game->board[34].data.property.has_hotel = 0;
  game->board[34].data.property.is_mortgaged = 0;
  game->board[34].data.property.is_insured = 0;
  game->board[34].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 35: Jaffna Railway Station
  // -------------------------------------------------------------
  game->board[35].index = 35;
  game->board[35].type = SQUARE_RAILWAY;
  strcpy(game->board[35].name, "Jaffna Railway Station");
  game->board[35].data.railway.price = 2000;
  game->board[35].data.railway.mortgage_value = 1000;
  game->board[35].data.railway.owner_id = -1;
  game->board[35].data.railway.is_mortgaged = 0;
  game->board[35].data.railway.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 36: National Event Card
  // -------------------------------------------------------------
  game->board[36].index = 36;
  game->board[36].type = SQUARE_EVENT;
  strcpy(game->board[36].name, "National Event Card");

  // -------------------------------------------------------------
  // Index 37: Nuwara Eliya
  // -------------------------------------------------------------
  game->board[37].index = 37;
  game->board[37].type = SQUARE_PROPERTY;
  strcpy(game->board[37].name, "Nuwara Eliya");
  game->board[37].data.property.group = GROUP_DARK_BLUE;
  game->board[37].data.property.price = 10000;
  game->board[37].data.property.base_rent = 1000;
  game->board[37].data.property.mortgage_value = 5000;
  game->board[37].data.property.house_cost = 3000;
  game->board[37].data.property.hotel_cost = 12000;
  game->board[37].data.property.owner_id = -1;
  game->board[37].data.property.num_houses = 0;
  game->board[37].data.property.has_hotel = 0;
  game->board[37].data.property.is_mortgaged = 0;
  game->board[37].data.property.is_insured = 0;
  game->board[37].data.property.is_loan_locked = 0;

  // -------------------------------------------------------------
  // Index 38: Bank of Ceylon
  // -------------------------------------------------------------
  game->board[38].index = 38;
  game->board[38].type = SQUARE_BANK;
  strcpy(game->board[38].name, "Bank of Ceylon");

  // -------------------------------------------------------------
  // Index 39: Galle Face
  // -------------------------------------------------------------
  game->board[39].index = 39;
  game->board[39].type = SQUARE_PROPERTY;
  strcpy(game->board[39].name, "Galle Face");
  game->board[39].data.property.group = GROUP_DARK_BLUE;
  game->board[39].data.property.price = 12000;
  game->board[39].data.property.base_rent = 1200;
  game->board[39].data.property.mortgage_value = 5000;
  game->board[39].data.property.house_cost = 3000;
  game->board[39].data.property.hotel_cost = 12000;
  game->board[39].data.property.owner_id = -1;
  game->board[39].data.property.num_houses = 0;
  game->board[39].data.property.has_hotel = 0;
  game->board[39].data.property.is_mortgaged = 0;
  game->board[39].data.property.is_insured = 0;
  game->board[39].data.property.is_loan_locked = 0;

  // Initialize depreciation fields for all properties
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY) {
      game->board[i].data.property.prop_age = 0;
      game->board[i].data.property.depreciation_pct = 0.0;
      game->board[i].data.property.base_rent_original = game->board[i].data.property.base_rent;
      game->board[i].data.property.building_condition = 100.0;
      game->board[i].data.property.rounds_without_maint = 0;
      game->board[i].data.property.has_structural_damage = 0;
    }
  }

  // Initialize Inflation / Economy state
  game->current_inflation_rate = 0.0;
  game->current_interest_rate = 0.08;

  game->railway_rent_base[0] = 250;
  game->railway_rent_base[1] = 500;
  game->railway_rent_base[2] = 1000;
  game->railway_rent_base[3] = 2000;

  game->utility_rent_base[0] = 4;
  game->utility_rent_base[1] = 10;
}
