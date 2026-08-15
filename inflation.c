#include "types.h"
#include <stdio.h>
#include <stdlib.h>

void handle_inflation(GameState *game) {
  // 1. Generate random inflation rate
  // Possible rates: -3%, 0%, 2%, 5%, 8%, 12%
  int rates[] = {-3, 0, 2, 5, 8, 12};
  int num_rates = sizeof(rates) / sizeof(rates[0]);
  int random_index = rand() % num_rates;
  int selected_rate_pct = rates[random_index];

  game->current_inflation_rate = (double)selected_rate_pct / 100.0;
  double multiplier = 1.0 + game->current_inflation_rate;

  // 2. Map inflation to baseline interest rate
  if (selected_rate_pct >= 8) {
    game->current_interest_rate = 0.12;
  } else if (selected_rate_pct >= 2) {
    game->current_interest_rate = 0.10;
  } else {
    game->current_interest_rate = 0.08;
  }

  // Announce the inflation cycle
  printf("\n======================================================\n");
  printf("[INFLATION CYCLE] A new economic phase has begun!\n");
  printf("Inflation Rate: %d%%\n", selected_rate_pct);
  printf("New Baseline Interest Rate: %.0lf%%\n", game->current_interest_rate * 100);
  printf("All property values, building costs, and rents have been adjusted.\n");
  printf("======================================================\n\n");

  // 3. Apply compounding formula to properties, railways, utilities
  for (int i = 0; i < TOTAL_SQUARES; i++) {
    if (game->board[i].type == SQUARE_PROPERTY) {
      game->board[i].data.property.price *= multiplier;
      game->board[i].data.property.base_rent *= multiplier;
      game->board[i].data.property.base_rent_original *= multiplier;
      game->board[i].data.property.house_cost *= multiplier;
      game->board[i].data.property.hotel_cost *= multiplier;
      game->board[i].data.property.mortgage_value *= multiplier;
    } else if (game->board[i].type == SQUARE_RAILWAY) {
      game->board[i].data.railway.price *= multiplier;
      game->board[i].data.railway.mortgage_value *= multiplier;
    } else if (game->board[i].type == SQUARE_UTILITY) {
      game->board[i].data.utility.price *= multiplier;
      game->board[i].data.utility.mortgage_value *= multiplier;
    }
  }

  // 4. Apply compounding formula to global rent arrays
  for (int i = 0; i < 4; i++) {
    game->railway_rent_base[i] *= multiplier;
  }

  for (int i = 0; i < 2; i++) {
    game->utility_rent_base[i] *= multiplier;
  }
}
