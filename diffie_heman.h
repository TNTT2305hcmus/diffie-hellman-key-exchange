#pragma once bigint.h
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

    // Toán tử cộng - trừ - nhân - mod
    BigInt operator+(const BigInt &other) const;
    BigInt operator-(const BigInt &other) const;
    BigInt operator*(const BigInt &other) const;
    BigInt operator/(const BigInt &other) const;
    BigInt operator%(const BigInt &mod) const;

    // Toán tử I/O
    friend ostream &operator<<(ostream &os, const BigInt &data);

    // Hàm modular_exponentiation
    static BigInt modular_exponentiation(BigInt base, BigInt exp, const BigInt &mod);

    static BigInt generate_private_key(BigInt p);

};
