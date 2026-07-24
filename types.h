#define TOTAL_SQUARES 40

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

typedef struct {
    int id;
    char name[50];
    SquareType type;
}BoardSquare;

typedef struct {
    double purchase_price;
    double mortgage_value;
    double base_rent;
    double house_construction_cost;
    double hotel_construction_cost;
    char current_owner[50];
    int mortgage_status = 0;
    int insurance_status = 0;
    int number_of_buildings;
}Property;

typedef 

