#include <stdio.h>
#include <string.h>
#include "types.h"

Player players[4];

void init_players(void){
    for (int i =0; i< 4; i++) {
        players[i].id = i;
        sprintf(players[i].name, "Player %d", i + 1);
        players[i].cash = 30000;
        players[i].current_position = 0;
        players[i].in_jail = 0;
        players[i].strategy = STRATEGY_CONSERVATIVE;
        players[i].owned_properties_count = 0;
        players[i].owned_railways_count = 0;
        players[i].owned_utilities_count = 0;
    }
}
