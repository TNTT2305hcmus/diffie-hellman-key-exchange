// Thư viện cơ bản của C
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
// Thư viện xử lý số nguyên lớn
#include <gmp.h>

// Thư viện hỗ trợ tạo seed entropy tùy theo hệ điều hành chạy
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

// Hàm hỗ trợ cho việc sinh số ngẫu nhiên Secure Random Generators (CSPRNG)
void get_secure_seed(gmp_randstate_t state)
{
    // Khởi tạo trạng thái RNG mặc định (Mersenne Twister)
    gmp_randinit_default(state);

    uint64_t seed = 0;

    // Dùng API để thu thập "entropy" (seed) từ hệ điều hành
#ifdef _WIN32 // Nếu chạy trên hệ điều hành Windows sẽ thực hiện đoạn code này
    // Sử dụng hàm BCryptGenRandom() để lấy entropy đưa vào seed
    if (BCryptGenRandom(NULL, (PUCHAR)&seed, sizeof(seed), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
    {
        fprintf(stderr, "[!] Lấy entropy không thành côn.\n");
        exit(1);
    }
#else // Nếu chạy trên hệ điều hành Linux sẽ thực hiện đoạn code này
    // Mở /dev/urandom để đọc dữ liệu ngẫu nhiên.
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
    {
        perror("[!] Truy cập /dev/urandom không thành công.");
        exit(1);
    }
    if (read(fd, &seed, sizeof(seed)) != sizeof(seed))
    {
        perror("[!] Lấy entropy từ /dev/urandom không thành công.");
        close(fd);
        exit(1);
    }
    // Đóng đầu đọc
    close(fd);
#endif
    // Nạp seed vào state
    gmp_randseed_ui(state, seed);
}

// Hàm sinh khóa bí mật
void generate_private_key(mpz_t private_key, const mpz_t p)
{
    /*
        Ta cần tạo khóa bí mật có giá trị trong khoảng từ [2, p-2] n
    */

    // Khởi tạo các biến dùng struct trong thư viện gmp
    mpz_t p_3;
    gmp_randstate_t state;

    mpz_init(p_3);         // Cấp phát bộ nhớ
    mpz_sub_ui(p_3, p, 3); // p-3

    get_secure_seed(state);

    mpz_urandomm(private_key, state, p_3);   // Hàm trả về một số ngẫu nhiên trong khoảng từ [0, p-4] do mod cho p-3
    mpz_add_ui(private_key, private_key, 2); // Cộng 2 để thu được số nằm trong khoảng từ [2, p-2]

    // Giải phóng bộ nhớ
    mpz_clear(p_3);
    gmp_randclear(state);
}

// Hàm main test kết quả
/*
int main()
{
    mpz_t p, private_key;
    mpz_init_set_str(p, "1347634825749823750293847502938475092384750923847509238475092384750923847509238475092384750923847509238475092384750923847509238475092384750", 10); // gán p = 100
    mpz_init(private_key);                                                                                                                                                  // khởi tạo private_key

    generate_private_key(private_key, p);

    gmp_printf("Private key: %Zd\n", private_key);

    mpz_clear(private_key);
    mpz_clear(p);

    return 0;
}
*/
/* Mở Ubuntu cài thư viện:  sudo apt update
                            sudo apt install libgmp-dev
*/
//  gcc generate_private_key.c -o main -lgmp
//  ./main
