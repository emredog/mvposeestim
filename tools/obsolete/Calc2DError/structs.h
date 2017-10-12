#ifndef STRUCTS
#define STRUCTS

struct Position
{
    double x;
    double y;

    Position(){this->x = -1.0; this->y = -1.0;}
    Position(double x, double y){this->x = x; this->y = y;}
};

#endif // STRUCTS

