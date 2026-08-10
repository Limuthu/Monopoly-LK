#include "types.h"
#include <stdio.h>

// Forward declarations
void should_buy(GameState *game, int player_id, int square_index);
void should_buy_railway(GameState *game, int player_id, int square_index);
void should_buy_utility(GameState *game, int player_id, int square_index);
void pay_property_rent(GameState *game, int player_index, int square_index);
void pay_railway_rent(GameState *game, int player_index, int square_index);
void pay_utility_rent(GameState *game, int player_index, int square_index, int dice_roll);
int find_player_index(GameState *game, int player_id);

void handle_landing(GameState *game, int player_position, int player_id, int dice_roll) {

  int visitor_index = find_player_index(game, player_id);

  switch (game->board[player_position].type) {

  case SQUARE_START:
    printf("Landed on GO\n");
    break;

  case SQUARE_PROPERTY:
    printf("Landed on %s\n", game->board[player_position].name);

    if (game->board[player_position].owner_id == -1) {
      should_buy(game, player_id, player_position);
    } else if (game->board[player_position].owner_id == player_id) {
      printf("%s owns this property\n", game->players[visitor_index].name);
    } else {
      pay_property_rent(game, visitor_index, player_position);
    }
    break;

  case SQUARE_RAILWAY:
    printf("Landed on %s\n", game->board[player_position].name);

    if (game->board[player_position].owner_id == -1) {
      should_buy_railway(game, player_id, player_position);
    } else if (game->board[player_position].owner_id == player_id) {
      printf("%s owns this railway\n", game->players[visitor_index].name);
    } else {
      pay_railway_rent(game, visitor_index, player_position);
    }
    break;

  case SQUARE_UTILITY:
    printf("Landed on %s\n", game->board[player_position].name);

    if (game->board[player_position].owner_id == -1) {
      should_buy_utility(game, player_id, player_position);
    } else if (game->board[player_position].owner_id == player_id) {
      printf("%s owns this utility\n", game->players[visitor_index].name);
    } else {
      pay_utility_rent(game, visitor_index, player_position, dice_roll);
    }
    break;

  case SQUARE_TAX:
    printf("Landed on Tax\n");
    break;
  case SQUARE_INSURANCE:
    printf("Landed on Insurance\n");
    break;
  case SQUARE_EVENT:
    printf("Landed on Event\n");
    break;
  case SQUARE_JAIL:
    printf("Landed on Jail\n");
    break;
  case SQUARE_SPECIAL:
    printf("Landed on Special\n");
    break;
  case SQUARE_BANK:
    printf("Landed on Bank\n");
    break;
  }
}