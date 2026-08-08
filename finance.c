#include "types.h"
#include <stdio.h>

void purchase_property(GameState *game, int player_id, int square_index) {
  if (game->board[square_index].owner_id == -1) {
    game->players[player_id].money -=
        game->board[square_index].data.property.price;
    game->board[square_index].owner_id = game->players[player_id].id;
    printf("%s purchased %s for LKR %.0lf\n", game->players[player_id].name,
           game->board[square_index].name,
           game->board[square_index].data.property.price);
  }
}
