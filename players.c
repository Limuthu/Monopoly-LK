#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void run_auction(GameState *game, int square_index);

// PLAYER INITIALIZATION

void init_players(GameState *game) {

  game->num_players = 4;

  // Player 1 - Aggressive Investor
  game->players[0].id = 1;
  snprintf(game->players[0].name, sizeof(game->players[0].name),
           "Aggressive Investor");
  game->players[0].money = 30000;
  game->players[0].position = 0;
  game->players[0].in_jail = 0;
  game->players[0].jail_turns = 0;
  game->players[0].roll_result = 0;
  game->players[0].has_loan = 0;
  game->players[0].loan_amount = 0;
  game->players[0].loan_rounds_left = 0;
  game->players[0].loan_start_round = 0;
  game->players[0].is_bankrupt = 0;

  // Player 2 - Conservative Banker
  game->players[1].id = 2;
  snprintf(game->players[1].name, sizeof(game->players[1].name),
           "Conservative Banker");
  game->players[1].money = 30000;
  game->players[1].position = 0;
  game->players[1].in_jail = 0;
  game->players[1].jail_turns = 0;
  game->players[1].roll_result = 0;
  game->players[1].has_loan = 0;
  game->players[1].loan_amount = 0;
  game->players[1].loan_rounds_left = 0;
  game->players[1].loan_start_round = 0;
  game->players[1].is_bankrupt = 0;

  // Player 3 - Risk Taker
  game->players[2].id = 3;
  snprintf(game->players[2].name, sizeof(game->players[2].name), "Risk Taker");
  game->players[2].money = 30000;
  game->players[2].position = 0;
  game->players[2].in_jail = 0;
  game->players[2].jail_turns = 0;
  game->players[2].roll_result = 0;
  game->players[2].has_loan = 0;
  game->players[2].loan_amount = 0;
  game->players[2].loan_rounds_left = 0;
  game->players[2].loan_start_round = 0;
  game->players[2].is_bankrupt = 0;

  // Player 4 - Opportunistic Trader
  game->players[3].id = 4;
  snprintf(game->players[3].name, sizeof(game->players[3].name),
           "Opportunistic Trader");
  game->players[3].money = 30000;
  game->players[3].position = 0;
  game->players[3].in_jail = 0;
  game->players[3].jail_turns = 0;
  game->players[3].roll_result = 0;
  game->players[3].has_loan = 0;
  game->players[3].loan_amount = 0;
  game->players[3].loan_rounds_left = 0;
  game->players[3].loan_start_round = 0;
  game->players[3].is_bankrupt = 0;

  for (int i = 0; i < game->num_players; i++) {
    game->players[i].passed_go_this_round = 0;
    game->players[i].active_national_card = NATIONAL_CARD_NONE;
    game->players[i].national_card_rounds_left = 0;
    game->players[i].revalued_group = GROUP_NONE;
    game->players[i].construction_suspended = 0;
  }
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

void roll_two_dice(int *d1, int *d2) {
  *d1 = ((rand() % 6) + 1);
  *d2 = ((rand() % 6) + 1);
}

void move_player(GameState *game, int player_index, int dice_roll) {
  int current_position = game->players[player_index].position;
  int new_position = (current_position + dice_roll) % TOTAL_SQUARES;
  game->players[player_index].position = new_position;
  printf("%s moves from %s to %s (position %d)\n",
         game->players[player_index].name, game->board[current_position].name,
         game->board[new_position].name, game->players[player_index].position);
  if (current_position > new_position && !game->players[player_index].in_jail) {
    game->players[player_index].money += 2000;
    game->players[player_index].passed_go_this_round = 1;
    printf("%s passed Go and collected LKR 2000\n", game->players[player_index].name);
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
double get_dynamic_mortgage(GameState *game, int square_index);

int is_regionally_buffed(GameState *game, int square_index) {
  if (game->board[square_index].type != SQUARE_PROPERTY) return 0;
  PropertyGroup group = game->board[square_index].data.property.group;
  if (game->active_regional_card == CARD_PORT_CITY_EXPANSION && (square_index == 1 || square_index == 3)) return 1;
  if (game->active_regional_card == CARD_IT_INDUSTRY_GROWTH && group == GROUP_PINK) return 1;
  if (game->active_regional_card == CARD_NORTHERN_DEV_PROGRAMME && group == GROUP_GREEN) return 1;
  if (game->active_regional_card == CARD_TEA_EXPORT_BOOM && square_index == 37) return 1;
  if (game->active_regional_card == CARD_UNIVERSITY_CITY_GROWTH && (square_index == 21 || square_index == 23)) return 1;
  if (game->active_regional_card == CARD_SOUTHERN_TOURISM_BOOM && group == GROUP_YELLOW) return 1;
  if (game->active_regional_card == CARD_AIRPORT_EXPANSION && group == GROUP_ORANGE) return 1;
  return 0;
}

// PLAYER BUYING DECISIONS

int should_buy(GameState *game, int player_id, int square_index) {
  int player_index = find_player_index(game, player_id);
  if (player_index == -1) return 0; 

  // Risk Taker
  if (player_id == 3) {

    // buys every property he lands on

    if (game->players[player_index].money >=
        game->board[square_index].data.property.price) {
      purchase_property(game, player_index, square_index);
    }else if(game->players[player_index].money < game->board[square_index].data.property.price) {
      printf("%s cannot afford to buy this property\n", game->players[player_index].name);
      run_auction(game, square_index);
    }

    // Conservative Banker
  } else if (player_id == 2) {

    // Priority 01 : Don't buy property in an economic recession event
    // Priortiy 02 : Purchases properties only if at least 50% of current cash remains after purchase

    if (game->active_economic_event == EVENT_ECONOMIC_RECESSION) {
      printf("[Conservative Banker] Refusing to buy %s due to Economic Recession!\n", game->board[square_index].name);
      run_auction(game, square_index);
      return 0;
    }

    if (game->board[square_index].data.property.price <=
        game->players[player_index].money * 0.5) {
      purchase_property(game, player_index, square_index);
    }else if(game->board[square_index].data.property.price > game->players[player_index].money * 0.5){
      printf("%s cannot afford to buy this property\n", game->players[player_index].name);
      run_auction(game, square_index);
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
      run_auction(game, square_index);

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


    // Priority 1 : If his money goes down to 500 after buying, he skips it.
    // Priority 2: Purchases if regionally buffed
    // Priority 3 : ROI is bad - skip
    // Priority 4 : One property away from completing the color group - buy
    // Priority 5 : Already owns 2+ properties in the group - buy
    // Priority 6 : Property is too expensive (price > 5000) - skip
    // Priority 7 : Default - buy anything reasonably priced

    if (game->players[player_index].money - game->board[square_index].data.property.price < 500) {
      printf("%s cannot afford to buy this property (reserve mindset)\n", game->players[player_index].name);
      run_auction(game, square_index);
    
    } else if (is_regionally_buffed(game, square_index)) {
      printf("%s purchases %s due to projected appreciation from Regional Development!\n", game->players[player_index].name, game->board[square_index].name);
      purchase_property(game, player_index, square_index);
      
    } else if (game->board[square_index].data.property.base_rent * 100 < game->board[square_index].data.property.price * 7) {
      printf("%s skips: ROI is bad (rent/price ratio too low)\n", game->players[player_index].name);
      run_auction(game, square_index);
    
    } else if (owned + 1 == size) {
      purchase_property(game, player_index, square_index);
    
    } else if (owned >= 2) {
      purchase_property(game, player_index, square_index);
    
    } else if (game->board[square_index].data.property.price > 5000) {
      printf("%s skips: property too expensive with no color-group advantage\n", game->players[player_index].name);
      run_auction(game, square_index);
    
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

  // Risk Taker - buys every railway he can afford
  if (player_id == 3) {
    if (game->players[player_index].money >= price) {
      purchase_railway(game, player_index, square_index);
    } else {
      printf("%s cannot afford to buy this railway\n", game->players[player_index].name);
      run_auction(game, square_index);
    }

  // Conservative Banker 
  // Priority 1: If in economic recession he skips buying
  // Priortiy 2 : buys only if at least 50% cash remains
  } else if (player_id == 2) {
    if (game->active_economic_event == EVENT_ECONOMIC_RECESSION) {
      printf("%s Refusing to buy railway %s due to Economic Recession!\n",  game->players[player_index].name,game->board[square_index].name);
      run_auction(game, square_index);
      return 0;
    }
    if (price <= game->players[player_index].money * 0.5) {
      purchase_railway(game, player_index, square_index);
    } else {
      printf("%s cannot afford to buy this railway\n", game->players[player_index].name);
      run_auction(game, square_index);
    }

  // Aggressive Investor - always buys railways as long as Rs.100 reserve is maintained
  } else if (player_id == 1) {
    if (game->players[player_index].money - price < 100) {
      printf("%s cannot afford to buy this railway (Future rent covering mindset)\n",
             game->players[player_index].name);
      run_auction(game, square_index);
    } else {
      purchase_railway(game, player_index, square_index);
    }

  // Opportunistic Trader 
  // Priority 1: If regulation active buys
  // Priority 2: Buys railway if Rs.500 reserve is maintained
  } else if (player_id == 4) {

    int reserve_needed;
    if (game->active_regulation == REGULATION_RAILWAY_MODERNIZATION) {
      reserve_needed = 0;
    } else {
      reserve_needed = 500;
    }

    if (game->players[player_index].money - price < reserve_needed) {
      printf("%s cannot afford to buy this railway (reserve mindset)\n", game->players[player_index].name);
      run_auction(game, square_index);
    } else {
      if (reserve_needed == 0) printf("%s buying railway due to Railway Modernization!\n", game->players[player_index].name);
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

  // Risk Taker - buys every utility he can afford
  if (player_id == 3) {
    if (game->players[player_index].money >= price) {
      purchase_utility(game, player_index, square_index);
    } else {
      printf("%s cannot afford to buy this utility\n", game->players[player_index].name);
      run_auction(game, square_index);
    }

  // Conservative Banker - buys only if at least 50% cash remains
  } else if (player_id == 2) {
    if (game->active_economic_event == EVENT_ECONOMIC_RECESSION) {
      printf("%s Refusing to buy utility %s due to Economic Recession!\n", game->players[player_index].name, game->board[square_index].name);
      run_auction(game, square_index);
      return 0;
    }
    if (price <= game->players[player_index].money * 0.5) {
      purchase_utility(game, player_index, square_index);
    } else {
      printf("%s cannot afford to buy this utility\n", game->players[player_index].name);
      run_auction(game, square_index);
    }

  // Aggressive Investor - always buys utilities (dice-multiplier income is strong) as long as Rs.100 reserve is maintained
  } else if (player_id == 1) {
    if (game->players[player_index].money - price < 100) {
      printf("%s cannot afford to buy this utility (Future rent covering mindset)\n", game->players[player_index].name);
      run_auction(game, square_index);
    } else {
      purchase_utility(game, player_index, square_index);
    }

  // Opportunistic Trader - buys utility if Rs.500 reserve is maintained, aggressively buys if Electricity Tariff Revision is active
  } else if (player_id == 4) {
    int reserve_needed;
    if (game->active_regulation == REGULATION_ELECTRICITY_TARIFF) {
      reserve_needed = 0;
    } else {
      reserve_needed = 500;
    }

    if (game->players[player_index].money - price < reserve_needed) {
      printf("%s cannot afford to buy this utility (reserve mindset)\n", game->players[player_index].name);
      run_auction(game, square_index);
    } else {
      if (reserve_needed == 0) printf("%s Aggressively buying utility due to Electricity Tariff Revision!\n", game->players[player_index].name);
      purchase_utility(game, player_index, square_index);
    }
  }
  return 0;
}
