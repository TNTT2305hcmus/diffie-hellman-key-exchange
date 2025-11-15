#include <iostream>
#include "diffie_heman.h"
#include "diffie_heman.cpp"
using namespace std;

int main(int argc, char **argv)
{
    // Kiểm tra tham số dòng lệnh
    int bit_size;
    if (argc < 2)
        bit_size = 512; // Mặc định 512 bits
    else
        bit_size = atoi(argv[1]);

    // Thiết lập các tham số ban đầu:
    //      Lấy số nguyên tố an toàn p
    //      Phần tử sinh g = 5
    BigInt p = BigInt::generate_safe_prime(bit_size);
    BigInt g = 5;

    // Sinh khóa riêng cho Alice và Bob
    BigInt a = BigInt::generate_private_key(p);
    BigInt b = BigInt::generate_private_key(p);

    // Tính khóa bí mật chung
    BigInt alice_shared_secret = BigInt::modular_exponentiation(g, a, p);
    BigInt bob_shared_secret = BigInt::modular_exponentiation(g, b, p);

    // In ra kết quả
    cout << "The shared secret that Alice claims: " << alice_shared_secret << endl;
    cout << "The shared secret that Bob claims: " << bob_shared_secret << endl;
    cout << "Check if both shared secrets are equal: " << (alice_shared_secret == bob_shared_secret ? "True" : "False") << endl;

    return 0;
}