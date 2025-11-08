#include <stdio.h>
#include <gmp.h>
/*
    Sử dụng kiểu dữ liệu mpz_t do thư viện GMP cung cấp 
    để phục vụ hệ thống xử lý số lớn 
    (chi tiết kiểu dữ liệu được nêu trong báo cáo)
*/

/*
    Hàm mpz_cmp_ui(a, b)
    Trong đó:
        - a: Số nguyên mang kiểu mpz_t
        - b: Số nguyên mang kiểu unsigned int
    --> So sánh a và b. Nếu kết quả trả về: 
        - (-1) --> a < b
        - (0) --> a = b
        - (1) --> a > b
*/

/*
    Mục tiêu trả về 1 biến mpz_t result với
        result = (base^exponent) mod (mod)
*/

void modular_exponentiation(mpz_t base, mpz_t exponent, mpz_t mod, mpz_t result){
   // Nếu mod = 1 hoặc mod = 0 thì trả về result = 0 luôn.
    if(mpz_cmp_ui(mod, 1) <= 0){
        mpz_set_ui(result, 0); 
        return;
    }

    /*
        Khởi tạo giá trị biến tạm:
            - e = exponent : Để chạy vòng lặp
            - temp = nullptr : 

    */
    mpz_t b, e, temp;
    mpz_init_set(e, exponent);
    mpz_init_set(b, base);
    mpz_init(temp);


    mpz_set_ui(result, 1); // result = 1
    mpz_mod(b, b, mod); // base = base % mod

    // while(e > 0):
    while(mpz_cmp_ui(e, 0) > 0){
        /*
            if(e % 2 == 1){
                temp = result * base;
                result = temp % mod;
            }
        */
        if(mpz_odd_p(e)){
            mpz_mul(temp, result, b);
            mpz_mod(result, temp, mod);
        }

        /*
            temp *= base;
            base = temp % mod;
            e /= 2;
        */
        mpz_mul(temp, b, b); 
        mpz_mod(b, temp, mod);
        mpz_fdiv_q_2exp(e, e, 1); // Phép dịch bit sang phải
    }

    // Giải phóng biến tạm
    mpz_clear(e);
    mpz_clear(temp);
}

int main(){
    mpz_t base, exponent, mod, result;
    mpz_init(base);
    mpz_init(exponent);
    mpz_init(mod);
    mpz_init(result);

    mpz_set_ui(base, 2);
    mpz_set_ui(exponent, 2); 
    mpz_set_ui(mod, 5);

    modular_exponentiation(base, exponent, mod, result);
    gmp_printf("(%Zd ^ %Zd) mod %Zd = %Zd\n", base, exponent, mod, result);

    mpz_clear(base);
    mpz_clear(exponent);
    mpz_clear(mod);
    mpz_clear(result);

    return 0;
}
