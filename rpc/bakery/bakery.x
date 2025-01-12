const ADD = 0;
const SUB = 1;
const MUL = 2;
const DIV = 3;

struct BAKERY
{
    int num;
    int op;
    double arg1;
    double arg2;
    double result;
};

program BAKERY_PROG
{
    version BAKERY_VER
    {
        struct BAKERY GET_NUM(struct BAKERY) = 1;
        struct BAKERY CALCULATOR_PROC(struct BAKERY) = 2;
    } = 1; /* Version number = 1 */
} = 0x20000001; /* RPC program number */