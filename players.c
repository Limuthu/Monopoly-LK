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

// Buy Decision

void purchase_property(GameState *game, int player_index, int square_index);
void purchase_railway(GameState *game, int player_index, int square_index);
void purchase_utility(GameState *game, int player_index, int square_index);

// Helpers
int count_owned_in_group(GameState *game, int player_id, int square_index);
int group_size(GameState *game, int square_index);
int find_player_index(GameState *game, int player_id);

int should_buy(GameState *game, int player_id, int square_index) {

  
  int player_index = -1;
  for (int i = 0; i < game->num_players; i++) {
    if (game->players[i].id == player_id) {
      player_index = i;
      break;
    }
  }
  if (player_index == -1) return 0; 


  // Risk Taker
  if (player_id == 3) {

    // buys every property he lands on

    if (game->players[player_index].money >=
        game->board[square_index].data.property.price) {
      purchase_property(game, player_index, square_index);
    }else if(game->players[player_index].money < game->board[square_index].data.property.price) {
      printf("%s cannot afford to buy this property\n", game->players[player_index].name);
    }

    // Conservative Banker
  } else if (player_id == 2) {

    // Purchases properties only if at least 50% of current cash remains after purchase

    if (game->board[square_index].data.property.price <=
        game->players[player_index].money * 0.5) {
      purchase_property(game, player_index, square_index);
    }else if(game->board[square_index].data.property.price > game->players[player_index].money * 0.5){
      printf("%s cannot afford to buy this property\n", game->players[player_index].name);
    }

    // Aggressive Investor
  } else if (player_id == 1) {

    // Owned property count in landed group
    int owned = count_owned_in_group(game, player_id, square_index);

    // Property count in each group
    int size = group_size(game, square_index);

    // if funds aren't remain to pay at least one future rent exit
    // Priority 1 : If he has only one property to buy to complete a color
    // Priority 2 : If he has started completing a color
    // Priority 3 : Purchase if the group is DARK BLUE
    // If not always purchase if all are okay

    if (game->players[player_index].money -
            game->board[square_index].data.property.price <
        100) {
      printf("%s cannot afford to buy this property (Future rent covering "
             "mindset)\n",
             game->players[player_index].name);

    } else if (owned + 1 == size) {
      purchase_property(game, player_index, square_index);

    } else if (owned >= 1) {
      purchase_property(game, player_index, square_index);

    } else if (game->board[square_index].data.property.group == GROUP_DARK_BLUE) {
      purchase_property(game, player_index, square_index);

    } else {
      purchase_property(game, player_index, square_index);
    }

    // Opportunistic Trader
  } else if (player_id == 4) {
    
    // Owned property count in landed group
    int owned = count_owned_in_group(game, player_id, square_index);

    // Property count in each group
    int size = group_size(game, square_index);

    // Auction preference: bid only if auction price is below listed market
    // value (i.e., the property went to auction at a discount). This is the
    // core opportunistic behaviour — never overpay, always exploit undervalued
    // lots.

    // Priority 1 : Cannot afford (must keep a cash reserve >= 500 after
    //              purchase to cover future rents) → skip
    // Priority 2 : ROI is bad (base_rent / price < 7%) → skip
    // Priority 3 : One property away from completing the color group → buy
    // Priority 4 : Already owns 2+ properties in the group → buy
    // Priority 5 : Property is too expensive (price > 5000) and none of the
    //              above conditions apply → skip
    // Priority 6 : Default — buy anything reasonably priced

    if (game->players[player_index].money - game->board[square_index].data.property.price < 500) {
      printf("%s cannot afford to buy this property (reserve mindset)\n", game->players[player_index].name);
    
    } else if (game->board[square_index].data.property.base_rent * 100 < game->board[square_index].data.property.price * 7) {
      printf("%s skips: ROI is bad (rent/price ratio too low)\n", game->players[player_index].name);
    
    } else if (owned + 1 == size) {
      purchase_property(game, player_index, square_index);
    
    } else if (owned >= 2) {
      purchase_property(game, player_index, square_index);
    
    } else if (game->board[square_index].data.property.price > 5000) {
      printf("%s skips: property too expensive with no color-group advantage\n", game->players[player_index].name);
    
    } else {
      purchase_property(game, player_index, square_index);
    }
  }
  return 0;
}

// RAILWAY BUY DECISION

int should_buy_railway(GameState *game, int player_id, int square_index) {

  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return 0;

  double price = game->board[square_index].data.railway.price;

  // Risk Taker — buys every railway he can afford
  if (player_id == 3) {
    if (game->players[player_index].money >= price) {
      purchase_railway(game, player_index, square_index);
    } else {
      printf("%s cannot afford to buy this railway\n", game->players[player_index].name);
    }

  // Conservative Banker — buys only if at least 50% cash remains
  } else if (player_id == 2) {
    if (price <= game->players[player_index].money * 0.5) {
      purchase_railway(game, player_index, square_index);
    } else {
      printf("%s cannot afford to buy this railway\n", game->players[player_index].name);
    }

  // Aggressive Investor — always buys railways (steady rent income)
  // as long as Rs.100 reserve is maintained
  } else if (player_id == 1) {
    if (game->players[player_index].money - price < 100) {
      printf("%s cannot afford to buy this railway (Future rent covering mindset)\n",
             game->players[player_index].name);
    } else {
      purchase_railway(game, player_index, square_index);
    }

  // Opportunistic Trader — buys railway if Rs.500 reserve is maintained
  } else if (player_id == 4) {
    if (game->players[player_index].money - price < 500) {
      printf("%s cannot afford to buy this railway (reserve mindset)\n",
             game->players[player_index].name);
    } else {
      purchase_railway(game, player_index, square_index);
    }
  }
  return 0;
}

// UTILITY BUY DECISION

int should_buy_utility(GameState *game, int player_id, int square_index) {

  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return 0;

  double price = game->board[square_index].data.utility.price;

  // Risk Taker — buys every utility he can afford
  if (player_id == 3) {
    if (game->players[player_index].money >= price) {
      purchase_utility(game, player_index, square_index);
    } else {
      printf("%s cannot afford to buy this utility\n", game->players[player_index].name);
    }

  // Conservative Banker — buys only if at least 50% cash remains
  } else if (player_id == 2) {
    if (price <= game->players[player_index].money * 0.5) {
      purchase_utility(game, player_index, square_index);
    } else {
      printf("%s cannot afford to buy this utility\n", game->players[player_index].name);
    }

  // Aggressive Investor — always buys utilities (dice-multiplier income is strong)
  // as long as Rs.100 reserve is maintained
  } else if (player_id == 1) {
    if (game->players[player_index].money - price < 100) {
      printf("%s cannot afford to buy this utility (Future rent covering mindset)\n",
             game->players[player_index].name);
    } else {
      purchase_utility(game, player_index, square_index);
    }

  // Opportunistic Trader — buys utility if Rs.500 reserve is maintained
  } else if (player_id == 4) {
    if (game->players[player_index].money - price < 500) {
      printf("%s cannot afford to buy this utility (reserve mindset)\n",
             game->players[player_index].name);
    } else {
      purchase_utility(game, player_index, square_index);
    }
  }
  return 0;
}
