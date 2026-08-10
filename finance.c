#include "types.h"
#include <stdio.h>

void purchase_property(GameState *game, int player_index, int square_index) {
  if (game->board[square_index].owner_id == -1) {
    game->players[player_index].money -=
        game->board[square_index].data.property.price;

    game->board[square_index].owner_id = game->players[player_index].id;
    game->board[square_index].data.property.owner_id =
        game->players[player_index].id;

    printf("%s purchased %s for LKR %.0lf\n", game->players[player_index].name,
           game->board[square_index].name,
           game->board[square_index].data.property.price);
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

void pay_property_rent(GameState *game, int player_index, int square_index) {
  if(game->board[square_index].data.property.is_mortgaged == 1) {
    printf("No rent, property is mortgaged\n");
    return;
  }

  int owner_id = game->board[square_index].owner_id;
  int owner_index = find_player_index(game, owner_id);

  double rent = game->board[square_index].data.property.base_rent;
  game->players[player_index].money -= rent;
  game->players[owner_index].money += rent;

  printf("%s paid LKR %.0lf property rent to %s for %s\n",
         game->players[player_index].name, rent,
         game->players[owner_index].name, game->board[square_index].name);
}

void pay_railway_rent(GameState *game, int player_index, int square_index) {
  if(game->board[square_index].data.railway.is_mortgaged == 1) {
    printf("No rent, railway is mortgaged\n");
    return;
  }

  int owner_id = game->board[square_index].owner_id;
  int owner_index = find_player_index(game, owner_id);

  int owned = count_owned_railways(game, owner_id);
  int rent = 0;
  
  switch(owned) {
    case 1: rent = 250;  break;
    case 2: rent = 500;  break;
    case 3: rent = 1000; break;
    case 4: rent = 2000; break;
  }

  game->players[player_index].money -= rent;
  game->players[owner_index].money += rent;

  printf("%s paid LKR %d railway rent to %s (%s owns %d station(s))\n",
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
  int rent = 0;

  switch(owned) {
    case 1: rent = 4  * dice_roll; break;
    case 2: rent = 10 * dice_roll; break;
  }
  
  game->players[player_index].money -= rent;
  game->players[owner_index].money += rent;

  printf("%s paid LKR %d utility rent to %s (dice was %d, %s owns %d utility(s))\n",
         game->players[player_index].name, rent,
         game->players[owner_index].name, dice_roll,
         game->players[owner_index].name, owned);
}
