#include "bigint.h"

int main()
{
    string a;
    cin >> a;

    BigInt bigA(a);
    BigInt b;
    int i;
    while (cin >> i)
    {
        BigInt b = BigInt::generate_private_key(bigA);
        cout << b << endl;
    }

    return 0;
}
