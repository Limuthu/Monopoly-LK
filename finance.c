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
