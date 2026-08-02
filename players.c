#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void roll_for_order(GameState *game) {
  srand((unsigned int)time(NULL)); // Seed RNG so rolls differ each run
  for (int i = 0; i < game->num_players; i++) {
    int dice = ((rand() % 6) + 1) + ((rand() % 6) + 1);
    game->players[i].roll_result = dice;
    // printf("Player %d rolled a %d\n", i + 1, game->players[i].roll_result);
  }
}

void sort_players(GameState *game) {
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
