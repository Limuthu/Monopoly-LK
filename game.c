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
void player_loan_decision(GameState *game, int player_id);
void handle_insurance_landing(GameState *game, int player_index);
void draw_national_card(GameState *game, int player_index);

void handle_landing(GameState *game, int player_position, int player_id, int dice_roll) {

  int visitor_index = find_player_index(game, player_id);

  switch (game->board[player_position].type) {

  case SQUARE_START:
    printf("Landed exactly on GO — LKR 2000 already collected\n");
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
    handle_insurance_landing(game, visitor_index);
    break;
  case SQUARE_EVENT:
    draw_national_card(game, visitor_index);
    break;
  case SQUARE_JAIL:
    printf("Landed on Jail\n");
    break;
  case SQUARE_SPECIAL:
    if (player_position == 30) {
      printf("Landed on Go To Jail!\n");
      printf("%s is sent directly to Jail without collecting Go money.\n", game->players[visitor_index].name);
      game->players[visitor_index].in_jail = 1;
      game->players[visitor_index].jail_turns = 0;
      game->players[visitor_index].position = 10;
    } else if (player_position == 10) {
      printf("Landed on Jail (Just Visiting)\n");
    } else {
      printf("Landed on %s\n", game->board[player_position].name);
    }
    break;
  case SQUARE_BANK:
    printf("Landed on Bank of Ceylon\n");
    player_loan_decision(game, player_id);
    break;
  }
}