#pragma once
#include <iostream>
#include <vector>
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <ctime>
#include <random>

using namespace std;

class BigInt
{
private:
    static const uint64_t BASE = (1ULL << 32); // 2^32: Cơ số (từng block trong data)
    vector<uint32_t> data;
    void trim(); // Xóa số "0" ở đầu

public:
    // Đây là 1 constructor tiện ích dùng để hỗ trợ khởi tạo các giá trị nhỏ
    // Vì unit64_t là kiểu dữ liệu lớn nhất được hỗ trợ nguyên bản
    // nên constructor này giúp ta chuyển đổi mọi hằng số nguyên C++ tiêu chuẩn sang dạng BigInt.
    BigInt(uint64_t initData = 0);

    // Với số nguyên lớn 512 bits.
    // Ta sẽ khởi tạo giá trị của nó từ chuỗi
    BigInt(const string &decString);

    // Định nghĩa các toán tử
    // Toán tử so sánh
    bool operator<(const BigInt &other) const;
    bool operator>(const BigInt &other) const;
    bool operator==(const BigInt &other) const;
    bool operator>=(const BigInt &other) const;

    // Toán tử cộng - trừ - nhân - chia - mod (native)
    BigInt operator+(const BigInt &other) const;
    BigInt operator-(const BigInt &other) const;
    BigInt operator*(const BigInt &other) const;
    BigInt operator/(const BigInt &other) const;
    BigInt operator%(const BigInt &mod) const;

    // Toán tử cộng - nhận (uint64_t)
    BigInt operator*(uint64_t small) const;
    BigInt operator+(uint64_t small) const;

    // Toán tử dịch bit
    BigInt operator>>(int shift) const;

    // Toán tử I/O
    friend ostream &operator<<(ostream &os, const BigInt &data);

    // Thuật toán nhân Karatsuba
    static BigInt karatsuba_multiply(const BigInt &a, const BigInt &b);
    // Phép nhân và mod
    static BigInt mod_mul(BigInt a, BigInt b, const BigInt &mod);
    // Thuật toán Barrett Mod
    static BigInt barrett_mod(const BigInt &a, const BigInt &mod);
    // Hàm modular_exponentiation
    static BigInt modular_exponentiation(BigInt base, BigInt exp, const BigInt &mod);

    // Hàm random bit
    static BigInt random_bits(int bits);
    // Hàm kiểm tra số nguyên tố (Áp dụng thuật toán Miller-Rabin)
    static bool is_prime_by_Miller_Rabin(const BigInt &n, int iterations = 7);
    // Hàm tạo số nguyên tố p
    static BigInt generate_prime(int bits = 512);
    // Hàm tạo số số nguyên tố an toàn
    static BigInt generate_safe_prime(int bits = 512);
    // Hàm sinh khóa riêng tư
    static BigInt generate_private_key(BigInt p);

};

