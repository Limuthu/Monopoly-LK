#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// PLAYER INITIALIZATION

void init_players(GameState *game) {

  game->num_players = 4;

  // Player 1 — Aggressive Investor
  game->players[0].id = 1;
  snprintf(game->players[0].name, sizeof(game->players[0].name),
           "Aggressive Investor");
  game->players[0].money = 30000;
  game->players[0].position = 0;
  game->players[0].in_jail = 0;
  game->players[0].jail_turns = 0;
  game->players[0].roll_result = 0;

  // Player 2 — Conservative Banker
  game->players[1].id = 2;
  snprintf(game->players[1].name, sizeof(game->players[1].name),
           "Conservative Banker");
  game->players[1].money = 30000;
  game->players[1].position = 0;
  game->players[1].in_jail = 0;
  game->players[1].jail_turns = 0;
  game->players[1].roll_result = 0;

  // Player 3 — Risk Taker
  game->players[2].id = 3;
  snprintf(game->players[2].name, sizeof(game->players[2].name), "Risk Taker");
  game->players[2].money = 30000;
  game->players[2].position = 0;
  game->players[2].in_jail = 0;
  game->players[2].jail_turns = 0;
  game->players[2].roll_result = 0;

  // Player 4 — Opportunistic Trader
  game->players[3].id = 4;
  snprintf(game->players[3].name, sizeof(game->players[3].name),
           "Opportunistic Trader");
  game->players[3].money = 30000;
  game->players[3].position = 0;
  game->players[3].in_jail = 0;
  game->players[3].jail_turns = 0;
  game->players[3].roll_result = 0;
}

// DICE ROLL

void roll_for_order(GameState *game) {
  for (int i = 0; i < game->num_players; i++) {
    int dice = ((rand() % 6) + 1) + ((rand() % 6) + 1);
    game->players[i].roll_result = dice;
  }
}

void reroll(Player *p1) {
  int dice = ((rand() % 6) + 1) + ((rand() % 6) + 1);
  p1->roll_result = dice;
}

int has_tie(GameState *game) {
  for (int i = 0; i < game->num_players - 1; i++) {
    if (game->players[i].roll_result == game->players[i + 1].roll_result) {
      /* printf("\nTie Between:\n%s & %s\n", game->players[i].name,
              game->players[i + 1].name);*/
      return 1;
    }
  }
  return 0;
}

void bubble_sort(GameState *game) {
  Player temp;
  for (int i = 0; i < game->num_players - 1; i++) {
    for (int j = 0; j < game->num_players - i - 1; j++) {
      if (game->players[j].roll_result < game->players[j + 1].roll_result) {
        temp = game->players[j];
        game->players[j] = game->players[j + 1];
        game->players[j + 1] = temp;
      }
    }
  }
}

void sort_players(GameState *game) {
  bubble_sort(game);
  while (has_tie(game)) {

    for (int i = 0; i < game->num_players - 1; i++) {
      if (game->players[i].roll_result == game->players[i + 1].roll_result) {
        reroll(&game->players[i]);
        printf("\n");
        printf("%s re-rolls %d\n", game->players[i].name,
               game->players[i].roll_result);
        reroll(&game->players[i + 1]);
        printf("%s re-rolls %d\n", game->players[i + 1].name,
               game->players[i + 1].roll_result);
      }
    }
    bubble_sort(game);
  }
}

// Player Movements

int roll_dice() {
  int dice1 = ((rand() % 6) + 1);
  int dice2 = ((rand() % 6) + 1);
  return dice1 + dice2;
}

void move_player(GameState *game, int player_index, int dice_roll) {
  int current_position = game->players[player_index].position;
  int new_position = (current_position + dice_roll) % TOTAL_SQUARES;
  game->players[player_index].position = new_position;
  printf("%s moves from %s to %s (position %d)\n",
         game->players[player_index].name, game->board[current_position].name,
         game->board[new_position].name, game->players[player_index].position);
  if (current_position > new_position) {
    printf("%s has passed Go\n", game->players[player_index].name);
  }
}
