#include "bigint.h"

int main()
{

    string a, b;
    cin >> a >> b;
    BigInt bigA(a);
    BigInt bigB(b);

    cout << "Phep cong: " << bigA + bigB << endl;
    cout << "Phep tru: " << bigA - bigB << endl;
    cout << "Phep nhan: " << bigA * bigB << endl;
    cout << "Phep mod: " << bigA % bigB << endl;

    cout << "Nhap mod and exp:" << endl;
    string c, d;
    cin >> c >> d;
    BigInt bigC(c);
    BigInt bigD(d);
    cout << "Tinh bigA^exp % mod: ";
    cout << BigInt::modular_exponentiation(bigA, bigD, bigC) << endl;
}
