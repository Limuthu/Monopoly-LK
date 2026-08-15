#include "types.h"
#include <stdio.h>

double get_dynamic_price(GameState *game, int square_index);
double get_dynamic_rent(GameState *game, int square_index);

void purchase_property(GameState *game, int player_index, int square_index) {
  if (game->board[square_index].owner_id == -1) {
    double purchase_price = get_dynamic_price(game, square_index);
    game->players[player_index].money -= purchase_price;

    game->board[square_index].owner_id = game->players[player_index].id;
    game->board[square_index].data.property.owner_id =
        game->players[player_index].id;

    printf("%s purchased %s for LKR %.0lf\n", game->players[player_index].name,
           game->board[square_index].name,
           purchase_price);
  }
}

void purchase_railway(GameState *game, int player_index, int square_index) {
  if (game->board[square_index].owner_id == -1) {
    game->players[player_index].money -=
        game->board[square_index].data.railway.price;

    game->board[square_index].owner_id = game->players[player_index].id;
    game->board[square_index].data.railway.owner_id =
        game->players[player_index].id;

    printf("%s purchased %s for LKR %.0lf\n", game->players[player_index].name,
           game->board[square_index].name,
           game->board[square_index].data.railway.price);
  }
}

void purchase_utility(GameState *game, int player_index, int square_index) {
  if (game->board[square_index].owner_id == -1) {
    game->players[player_index].money -=
        game->board[square_index].data.utility.price;

    game->board[square_index].owner_id = game->players[player_index].id;
    game->board[square_index].data.utility.owner_id =
        game->players[player_index].id;

    printf("%s purchased %s for LKR %.0lf\n", game->players[player_index].name,
           game->board[square_index].name,
           game->board[square_index].data.utility.price);
  }
}

int find_player_index(GameState *game, int player_id);
int count_owned_railways(GameState *game, int owner_id);
int count_owned_utilities(GameState *game, int owner_id);
int has_monopoly(GameState *game, int player_id, int square_index);

void pay_property_rent(GameState *game, int player_index, int square_index) {
  if(game->board[square_index].data.property.is_mortgaged == 1) {
    printf("No rent, property is mortgaged\n");
    return;
  }

  if (game->board[square_index].data.property.is_damaged == 1) {
    printf("No rent, property is damaged from a disaster and needs repairs!\n");
    return;
  }

  int owner_id = game->board[square_index].owner_id;
  int owner_index = find_player_index(game, owner_id);

  double base_rent = get_dynamic_rent(game, square_index);
  int num_houses = game->board[square_index].data.property.num_houses;
  int hotel = game->board[square_index].data.property.has_hotel;

  // Rent multiplier (Table 6)
  // No buildings: 1x | 1 house: 2x | 2 houses: 3x | 3 houses: 5x | 4 houses: 7x | Hotel: 10x

  double rent = base_rent;

  if (hotel) {
    rent = base_rent * 10;
  } else if (num_houses == 4) {
    rent = base_rent * 7;
  } else if (num_houses == 3) {
    rent = base_rent * 5;
  } else if (num_houses == 2) {
    rent = base_rent * 3;
  } else if (num_houses == 1) {
    rent = base_rent * 2;
  }

  // 1. Apply property depreciation to base_rent
  rent = rent * (1.0 - game->board[square_index].data.property.depreciation_pct / 100.0);

  // 2. Apply building condition multiplier
  double get_condition_multiplier(double condition);
  double condition_mult = get_condition_multiplier(game->board[square_index].data.property.building_condition);
  rent = rent * condition_mult;

  if (condition_mult == 0.0) {
    printf("No rent, building is closed due to poor condition!\n");
    return;
  }

  game->players[player_index].money -= rent;
  game->players[owner_index].money += rent;

  // Display what multiplier was applied
  if (hotel) {
    printf("%s paid LKR %.0lf rent to %s for %s (HOTEL - 10x)\n",
           game->players[player_index].name, rent,
           game->players[owner_index].name, game->board[square_index].name);
  } else if (num_houses > 0) {
    printf("%s paid LKR %.0lf rent to %s for %s (%d house(s))\n",
           game->players[player_index].name, rent,
           game->players[owner_index].name, game->board[square_index].name,
           num_houses);
  } else {
    printf("%s paid LKR %.0lf rent to %s for %s\n",
           game->players[player_index].name, rent,
           game->players[owner_index].name, game->board[square_index].name);
  }
}

void pay_railway_rent(GameState *game, int player_index, int square_index) {
  if(game->board[square_index].data.railway.is_mortgaged == 1) {
    printf("No rent, railway is mortgaged\n");
    return;
  }

  int owner_id = game->board[square_index].owner_id;
  int owner_index = find_player_index(game, owner_id);

  int owned = count_owned_railways(game, owner_id);
  double rent = 0;
  if (owned > 0 && owned <= 4) {
    rent = game->railway_rent_base[owned - 1];
  }
  
  if (game->active_economic_event == EVENT_FUEL_CRISIS) {
    rent *= 2.0; // Railway rent doubles
  }
  
  if (game->active_regional_card == CARD_TRANSPORT_STRIKE) {
    rent *= 0.60; // Railway revenue reduced by 40%
  }

  game->players[player_index].money -= rent;
  game->players[owner_index].money += rent;

  printf("%s paid LKR %.0lf railway rent to %s (%s owns %d station(s))\n",
         game->players[player_index].name, rent,
         game->players[owner_index].name, game->players[owner_index].name, owned);
}

void pay_utility_rent(GameState *game, int player_index, int square_index, int dice_roll) {
  if(game->board[square_index].data.utility.is_mortgaged == 1) {
    printf("No rent, utility is mortgaged\n");
    return;
  }

  int owner_id = game->board[square_index].owner_id;
  int owner_index = find_player_index(game, owner_id);

  int owned = count_owned_utilities(game, owner_id);
  double rent = 0;
  if (owned > 0 && owned <= 2) {
    double multiplier = game->utility_rent_base[owned - 1];
    rent = dice_roll * multiplier;
  }
  
  if (game->active_regional_card == CARD_ELECTRICITY_TARIFF) {
    rent *= 1.25; // Utility rent +25%
  }
  
  if (game->active_regional_card == CARD_WATER_SHORTAGE && square_index == 28) {
    rent *= 1.20; // Water utility revenue +20%
  }

  game->players[player_index].money -= rent;
  game->players[owner_index].money += rent;

  printf("%s paid LKR %.0lf utility rent to %s (dice was %d, %s owns %d utility(s))\n",
         game->players[player_index].name, rent,
         game->players[owner_index].name, dice_roll,
         game->players[owner_index].name, owned);
}
