#include <stdio.h>
#include <gmp.h>
#include <time.h>

// hàm sinh safe prime
// p: biến mpz_t để lưu kết quả
// bits: số bit mong muốn của safe prime
void generate_safe_prime(mpz_t p, int bits) 
{
    mpz_t q;
    mpz_inits(q, NULL); // khởi tạo biến q (tạm thời)

    // khởi tạo bộ sinh số ngẫu nhiên
    gmp_randstate_t state;
    gmp_randinit_mt(state);              // dùng thuật toán Mersenne Twister
    gmp_randseed_ui(state, time(NULL));  // seed là thời gian hiện tại

    // vòng lặp tìm safe prime
    do {
        // sinh số ngẫu nhiên (bits-1) bit
        mpz_urandomb(q, state, bits - 1);

        // chuyển q thành số nguyên tố tiếp theo ≥ q
        mpz_nextprime(q, q);

        // tính p = 2*q + 1
        mpz_mul_ui(p, q, 2);
        mpz_add_ui(p, p, 1);

        // kiểm tra p có phải nguyên tố không
        // mpz_probab_prime_p(p, 25) → kiểm tra probabilistic với 25 lần
        // nếu p không phải prime, tiếp tục lặp
    } while (!mpz_probab_prime_p(p, 25));

    // giải phóng bộ nhớ
    mpz_clears(q, NULL);
    gmp_randclear(state);
}

int main() 
{
    mpz_t prime;
    mpz_init(prime);

    generate_safe_prime(prime, 256);
    
    gmp_printf("Safe prime (256-bit): %Zd\n", prime);

    mpz_clear(prime);
    return 0;
}
