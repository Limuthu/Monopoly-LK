#define TOTAL_SQUARES 40

// Square types

typedef enum{
    SQUARE_START,
    SQUARE_PROPERTY,
    SQUARE_RAILWAY,
    SQUARE_UTILITY,
    SQUARE_TAX,
    SQUARE_EVENT,
    SQUARE_JAIL,
    SQUARE_SPECIAL,
    SQUARE_BANK,
    SQUARE_INSURANCE
}SquareType;

// Group types for properties

typedef enum {
    GROUP_NONE,
    GROUP_BROWN,
    GROUP_LIGHT_BLUE,
    GROUP_PINK,
    GROUP_ORANGE,
    GROUP_RED,
    GROUP_YELLOW,
    GROUP_GREEN,
    GROUP_DARK_BLUE,
}PropertyGroup;

// Property data structure

typedef struct {
    double price;
    double mortgage_value;
    double base_rent;
    double house_cost;
    double hotel_cost;
    int num_houses;
    int has_hotel;
    int owner_id;
    int is_mortgaged;
    int is_insured;
    PropertyGroup group;
}PropertyData;

// Railway data structure

typedef struct{
    double price;
    int owner_id;
    int is_mortgaged;
}RailwayData;

// utility data structure

typedef struct {
    double price;
    int owner_id;
    int is_mortgaged;
}UtilityData;

//Square data structure

typedef struct{
    int index;
    char name[30];
    SquareType type;
    union {
        PropertyData property;
        RailwayData railway;
        UtilityData utility;
    } data;
}Square;

//player

typedef enum {
    STRATEGY_CONSERVATIVE,
    STRATEGY_AGGRESSIVE,
    STRATEGY_RISK,
    STRATEGY_OPPORTUNISTIC
}StrategyType;

typedef struct{
    int id;
    char name[30];
    double cash;
    int current_position;
    int in_jail;
    StrategyType strategy;

    int owned_properties_count;
    int owned_railways_count;
    int owned_utilities_count;

}Player;

