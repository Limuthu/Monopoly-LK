#include "types.h"
#include <stdlib.h>
#include <time.h>

void run_game_simulation(void);

int main(void) {
  srand((unsigned int)time(NULL));

  run_game_simulation();

  return 0;
}