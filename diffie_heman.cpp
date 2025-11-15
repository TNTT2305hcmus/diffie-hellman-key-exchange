#include "diffie_heman.h"
// Loại bỏ các chữ số "0" vô nghĩa
/*
    @logic
    1. Kiểm tra data có rỗng không
    2. Nếu phần tử cuối của data là 0 thì loại bỏ nó
    3. Ví dụ:
        Số 4590 sẽ được lưu trữ --> data = [0, 9, 5, 4]
        Nếu data = [0, 9, 5, 4 , 0] --> Số = 04590 --> Số 0 ở đầu vô nghĩa
*/
void BigInt::trim()
{
    while (!data.empty() && data.back() == 0)
    {
        data.pop_back();
    }
}

/*
    @param initData (Giá trị khởi tạo mặc định)
    @logic
    1. Số 0 không cần được lưu trữ nên ta return, vector data rỗng
    2. Trích xuất khổi bit thấp
    OxFFFFFFFF là 1 hằng số hex (32 bit 1).
    Phép toán & cho phép
        - Giữ lại 32 bit thấp của initData (bit 0 -> 31)
        - Đặt 32 bit cao thành 0 (bit 32 - 63)
    --> Sau ép kiểu, ta thu được phần thấp của số 64 bits dưới kiểu dữ liệu unit32_t.
    Đây là block đầu tiên của data.
    3. Trích xuất khối bit cao
    Sau đó ta dịch phải 32 bit và ép kiểu để thu được block gồm 32 bit cao của số 64 bits
*/
BigInt::BigInt(uint64_t initData)
{
    if (initData == 0)
    {
        return;
    }
    data.push_back((uint32_t)(initData & 0xFFFFFFFF));
    if (initData >> 32)
    {
        data.push_back((uint32_t)(initData >> 32));
    }
}

/*
    @param decString (Chuỗi dec cho số lớn 512 bit)
    @logic
    1. Duyệt
    Lặp và kiểm tra từng ký tự. Nếu
        - Không phải số : bỏ qua
        - Là số: Cập nhật result
    2. Gán lại result cho *this (đối tượng hiện tại đang được khởi tạo)
*/
BigInt::BigInt(const string &decString)
{
    BigInt result;
    for (char c : decString)
    {
        if (!isdigit(c))
        {
            continue;
        }
        else
        {
            result = result * 10 + BigInt(c - '0');
        }
    }
    *this = result;
}

// Định nghĩa các toán tử

// Toán tử nhỏ hơn
/*
    @param other (Giá trị BigInt cần so sanh)
    @logic
    1. Nếu size khác, thì số nào có size lớn hơn sẽ lớn hơn
    2. Nếu size bằng nhau, thì ta so sánh từng giá trị trong BigInt
    3. So sánh từ giá trị cuối --> đầu (giá trị hàng lớn nhất trước)
*/
bool BigInt::operator<(const BigInt &other) const
{
    if (data.size() != other.data.size())
    {
        return data.size() < other.data.size();
    }
    for (int i = (int)data.size() - 1; i >= 0; i--)
    {
        if (data[i] != other.data[i])
        {
            return data[i] < other.data[i];
        }
    }
    return false;
}

bool BigInt::operator>(const BigInt &other) const
{
    return other < *this;
}

// Toán tử ==
bool BigInt::operator==(const BigInt &other) const
{
    return data == other.data;
}

// Toán tử >=
bool BigInt::operator>=(const BigInt &other) const
{
    return !(*this < other);
}

// Toán tử cộng
/*
    @param other (BigInt cần cộng vào)
    @logic
    1. Áp dụng quy tắc cộng 2 số nguyên thông thường
    2. Cộng từng chữ số theo vị trí tương ứng (cộng thêm carry)
    3. Sử dụng biến uint64_t carry làm số nhớ
*/
BigInt BigInt::operator+(const BigInt &other) const
{
    // Khởi tạo và gán result có size của số lớn hơn
    BigInt result;
    uint64_t carry = 0;
    size_t max_size = max(data.size(), other.data.size());
    result.data.resize(max_size);

    // Duyệt để cộng từng giá trị
    for (int i = 0; i < max_size; i++)
    {
        // Cộng carry vào trước
        uint64_t sum = carry;

        // Vì 2 số có thể khác size. Nên ta cần kiểm tra
        // Nếu i nhỏ hơn thì ta cộng, i lớn hơn xem như cộng với 0
        if (i < data.size())
        {
            sum += data[i];
        }
        if (i < other.data.size())
        {
            sum += other.data[i];
        }

        // Ta nhận 1 chữ số hàng đơn vị từ sum để cho vào result (nghĩa là 32 bit thấp)
        // Chữ số hàng tiếp theo ta lưu vào số nhớ để cộng với hàng cao hơn (32 bit cao)
        result.data[i] = (uint32_t)(sum & 0xFFFFFFFF);
        carry = sum >> 32;
    }

    // Sau khi cộng hết, nếu carry != 0, cập nhật thêm 1 chữ số hàng cao nhất cho result
    if (carry)
    {
        result.data.push_back((uint32_t)(carry));
    }
    result.trim();
    return result;
}

// Hỗ trợ trường hợp cộng số nguyên nhỏ, giúp tăng hiệu suất thay vì ép sang kiểu BigInt
BigInt BigInt::operator+(uint64_t small) const
{
    BigInt res = *this;

    uint32_t low = (uint32_t)small;
    uint32_t high = (uint32_t)(small >> 32);

    uint64_t sum = (uint64_t)res.data[0] + low;
    res.data[0] = (uint32_t)sum;
    uint64_t carry = sum >> 32;

    size_t i = 1;

    if (high || carry)
    {
        uint64_t sum2 = (uint64_t)res.data[1] + high + carry;
        res.data[1] = (uint32_t)sum2;
        carry = sum2 >> 32;
        i = 2;
    }

    while (carry)
    {
        if (i >= res.data.size())
            res.data.push_back(0);
        uint64_t s = (uint64_t)res.data[i] + carry;
        res.data[i] = (uint32_t)s;
        carry = s >> 32;
        i++;
    }

    res.trim();
    return res;
}

// Toán tử trừ
/*
    @param other (Giá trị BigInt muốn trừ đi)
    @logic
    1. Nếu thực hiện phép trừ ra kết quả âm (*this < other). Trả về lỗi và 0
    2. Mặc định *this >= other. Nên gắn size của result là *this.data
    3. Thực hiện phép trừ như trừ 2 số nguyên thông thường
    4. Biến carry để mượn
        4.1. carry = -1: Có mượn
        4.2. carry = 0: Chưa mượn
*/
BigInt BigInt::operator-(const BigInt &other) const
{
    if (*this < other)
    {
        cout << "Lỗi, không thể thực hiện phép trừ cho ra kết quả âm" << endl;
        return BigInt(0);
    }

    BigInt result;
    result.data.resize(data.size());
    int64_t carry = 0;

    for (int i = 0; i < data.size(); i++)
    {
        // Công thức:
        // diff = số hiện tại - số bị trừ + số mượn trước đó
        // Vì other có thể có size nhỏ hơn, nên cần kiểm tra chữ số i và other.data.size
        // Nếu i nhỏ hơn thì trừ số thuộc other
        // Nếu i >= xem như trừ cho 0
        int64_t diff = (int64_t)data[i] - (i < other.data.size() ? other.data[i] : 0) + carry;

        // Nếu mà diff < 0
        // Nghĩa là số trừ nhỏ hơn số bị trừ.
        // Ta cần mượn 1 chữ số hàng tiếp theo để trừ như phép trừ thông thường
        // Đánh dấu carry để trả lại khi xét hàng tiếp theo
        if (diff < 0)
        {
            diff += BASE;
            carry = -1;
        }
        else
        {
            carry = 0;
        }
        result.data[i] = (uint32_t)(diff);
    }
    result.trim();
    return result;
}

// Thuật toán nhân karatsuba
BigInt BigInt::karatsuba_multiply(const BigInt &a, const BigInt &b)
{
    if (a.data.size() < 64 || b.data.size() < 64)
    {
        BigInt result;
        result.data.assign(a.data.size() + b.data.size(), 0);
        for (size_t i = 0; i < a.data.size(); ++i)
        {
            uint64_t carry = 0;
            for (size_t j = 0; j < b.data.size(); ++j)
            {
                uint64_t cur = (uint64_t)result.data[i + j] + (uint64_t)a.data[i] * b.data[j] + carry;
                result.data[i + j] = (uint32_t)(cur & 0xFFFFFFFF);
                carry = cur >> 32;
            }
            size_t k = i + b.data.size();
            while (carry)
            {
                if (k >= result.data.size())
                    result.data.push_back(0);
                uint64_t cur = (uint64_t)result.data[k] + carry;
                result.data[k] = (uint32_t)(cur & 0xFFFFFFFF);
                carry = cur >> 32;
                ++k;
            }
        }
        result.trim();
        return result;
    }

    size_t n = max(a.data.size(), b.data.size());
    size_t m = n / 2;
    BigInt high1, low1, high2, low2;
    low1.data.assign(a.data.begin(), a.data.begin() + min(a.data.size(), m));
    if (a.data.size() > m)
        high1.data.assign(a.data.begin() + m, a.data.end());
    else
        high1 = BigInt(0);
    low2.data.assign(b.data.begin(), b.data.begin() + min(b.data.size(), m));
    if (b.data.size() > m)
        high2.data.assign(b.data.begin() + m, b.data.end());
    else
        high2 = BigInt(0);

    BigInt z0 = karatsuba_multiply(low1, low2);
    BigInt z1 = karatsuba_multiply(low1 + high1, low2 + high2);
    BigInt z2 = karatsuba_multiply(high1, high2);

    BigInt result;
    result.data.assign((n + 1) * 2, 0);
    for (size_t i = 0; i < z0.data.size(); ++i)
        result.data[i] += z0.data[i];
    BigInt temp = z1 - z2 - z0;
    for (size_t i = 0; i < temp.data.size(); ++i)
        result.data[i + m] += temp.data[i];
    for (size_t i = 0; i < z2.data.size(); ++i)
        result.data[i + 2 * m] += z2.data[i];

    uint64_t carry = 0;
    for (size_t i = 0; i < result.data.size(); ++i)
    {
        uint64_t cur = (uint64_t)result.data[i] + carry;
        result.data[i] = (uint32_t)(cur & 0xFFFFFFFF);
        carry = cur >> 32;
    }
    while (carry)
    {
        result.data.push_back((uint32_t)(carry & 0xFFFFFFFF));
        carry >>= 32;
    }
    result.trim();
    return result;
}

// Toán tử nhân
/*
    @param other (Giá trị BigInt nhân vào)
    @logic
    Xử lý trường hợp nhân tổng quát bằng thuật toán nhân karatsuba để tăng hiệu suất
*/
BigInt BigInt::operator*(const BigInt &other) const
{
    return karatsuba_multiply(*this, other);
}

// Xử lý thêm trường hợp nhân số nhỏ
/*
    Tóm tắt ý tưởng:
    - uint64_t small là một số 64 bit cần nhân
    - 1 block trong BigInt chỉ lưu 32 bit
    - Cần chia small là 2 phần low và high và nhân với BigInt
    - Tức là data(32 bits) * small(64 bits) = data(32 bits) * [low(32 bits) + high >> 32 (32 bits)]
*/
BigInt BigInt::operator*(uint64_t small) const
{
    BigInt res;
    if (small == 0)
        return BigInt(0);

    // Tách small thành 2 giá trị low và high, mỗi giá trị lần lượt chứa 32 bit thấp và 32 bit cao
    uint32_t low = (uint32_t)(small);
    uint32_t high = (uint32_t)(small >> 32);

    // Khi nhân vào, data sẽ tăng tối đa 2 block
    res.data.resize(data.size() + 2);

    uint64_t carry = 0; // Lưu giá trị nhớ từ phép toán với 64 bits sang 32 bits
    for (size_t i = 0; i < data.size(); i++)
    {
        // Thực hiện nhân data[i] với low và high (nhân 2 số 32 bit sẽ sinh ra số tối đa 64 bit)
        uint64_t p1 = (uint64_t)data[i] * low;
        uint64_t p2 = (uint64_t)data[i] * high;

        // Thực hiện theo thuật toán nhân 2 số không dấu

        uint64_t sum = p1 + carry + res.data[i];
        res.data[i] = (uint32_t)sum; // Lưu 32 bits thấp
        carry = sum >> 32;           // Giữ 32 bits cao

        uint64_t sum2 = (uint64_t)res.data[i + 1] + (p2 + (carry));
        res.data[i + 1] = (uint32_t)sum2; // Lưu 32 bits thấp
        carry = sum2 >> 32;               // Giữ 32 bits cao
    }

    size_t idx = data.size();

    // Xử lý carry còn thừa
    while (carry)
    {
        if (idx >= res.data.size())
            res.data.push_back((uint32_t)0); // Nếu còn thừa carry và các block đã full, tạo block mới

        uint64_t sum = (uint64_t)res.data[idx] + carry;
        res.data[idx] = (uint32_t)sum; // Lưu 32 bits thấp
        carry = sum >> 32;             // Giữ 32 bits cao
        idx++;
    }

    res.trim();
    return res;
}

// Toán tử chia
BigInt BigInt::operator/(const BigInt &other) const
{
    if (other == BigInt(0))
        throw runtime_error("Division by zero!");
    BigInt quotient, remainder;
    vector<uint32_t> quot_data;
    quot_data.reserve(data.size());
    remainder = BigInt(0);
    for (int i = (int)data.size() - 1; i >= 0; i--)
    {
        remainder = remainder * BASE + BigInt((uint64_t)data[i]);
        uint32_t q = 0, l = 0, r = 0xFFFFFFFFu;
        while (l <= r)
        {
            uint32_t m = l + ((r - l) >> 1);
            BigInt prod = other * BigInt((uint64_t)m);
            if (!(remainder < prod))
            {
                q = m;
                l = m + 1;
            }
            else
                r = m - 1;
        }
        quot_data.push_back(q);
        remainder = remainder - other * BigInt((uint64_t)q);
    }
    reverse(quot_data.begin(), quot_data.end());
    quotient.data = quot_data;
    quotient.trim();
    return quotient;
}

// Thuật toán barrett mod
BigInt BigInt::barrett_mod(const BigInt &a, const BigInt &mod)
{
    if (a < mod)
        return a;
    size_t k = mod.data.size();
    BigInt base_pow(0);
    base_pow.data.assign(2 * k + 1, 0);
    base_pow.data[2 * k] = 1;
    base_pow.trim();
    BigInt mu = base_pow / mod;

    int shift1 = (int)((k > 0 ? (k - 1) : 0) * 32);
    int shift2 = (int)((k + 1) * 32);
    BigInt q1 = a >> shift1;
    BigInt q2 = q1 * mu;
    BigInt q3 = q2 >> shift2;

    BigInt r = a - q3 * mod;
    while (!(r < mod))
    {
        r = r - mod;
    }
    return r;
}

// Toán tử mod
/*
    @param mod (Giá trị cần mod)
    Áp dụng thuật toán barrett mod để tăng hiệu suất toán tử mod
*/
BigInt BigInt::operator%(const BigInt &mod) const
{
    return barrett_mod(*this, mod);
}

// Hàm nhân mod áp dụng thừa kế thuật toán barrett mod để tăng hiệu suất
BigInt BigInt::mod_mul(BigInt a, BigInt b, const BigInt &mod)
{
    return barrett_mod(a * b, mod);
}

// Toán tử dịch bit sang phải
BigInt BigInt::operator>>(int shift) const
{
    // Nếu số bit cần dịch <= 0 thì không thực hiện dịch
    if (shift <= 0)
        return *this;

    // Tính tổng số bit của data hiện tại, nếu số bit dịch lớn hơn tổng thì trả về 0
    int totalBits = (int)data.size() * 32;
    if (shift >= totalBits)
        return BigInt(0);

    BigInt result;
    int full_blocks = shift / 32;                  // Tính số block cần xóa khi dịch
    int bit_shift = shift % 32;                    // Tính số bit cần dịch còn lại
    result.data.resize(data.size() - full_blocks); // Khởi tạo vùng nhớ mới

    // Ứng với mỗi block dữ liệu
    for (size_t i = 0; i < result.data.size(); ++i)
    {
        // Vì đã bỏ các block cần dịch full, block i sau khi dịch chính là block i + full_blocks trước khi dịch
        /*
            Vì BigInt biểu diễn dữ liệu từ phải sang trái, VD: [0, 5] tức là 50
            Nên khi dịch phải ta cần dữ liệu của block hiện tại và block bên phải của nó
        */

        // Lấy dữ liệu block hiện tại (32 bit thấp)
        uint64_t low = data[i + full_blocks]; // Lúc này có 32 bit đầu là 0 và 32 bit sau lưu giá trị của block

        // Lấy dữ liệu block tiếp theo, nếu có (32 bit cao)
        uint64_t high = 0;
        if (i + full_blocks + 1 < data.size())
            high = data[i + full_blocks + 1]; // Lúc này có 32 bit đầu là 0 và 32 bit sau lưu giá trị của block

        /*
            Gộp 32 bit cao và 32 bit thấp bằng cách:
            Lấy high dịch trái 32 bit => high có 32 bit đầu là giá trị của block sau và 32 bit cuối là 0
            Lấy kết quả OR với low (Vì bit nào OR với 0 cũng bằng chính nó)
            Ta thu được kết quả có 32 bit đầu là high và 32 bit cuối là low
        */
        uint64_t combined = (high << 32) | low;

        // Lấy giá trị đã gộp dịch phải số bit còn lại để đưa giá trị của block sau sang block trước
        // &OxFFFFFFFFu để đảm bảo chỉ lấy đúng 32 bit thấp
        // Lưu giá trị vào block
        result.data[i] = (uint32_t)((combined >> bit_shift) & 0xFFFFFFFFu);
    }
    result.trim();
    return result;
}

// Xuất
/*
    @param os (luồng xuất)
    @param number (Số BigInt cần xuất ra màn hình)
    @logic
    1. Nếu number.data rỗng --> Số 0
    2. Ý tưởng chính: Chia liên tục cho 10 và lấy phần dư làm chữ số
    3. Chuyển chữ số này sang chuỗi decString để xuất ra màn hình
*/
ostream &operator<<(ostream &os, const BigInt &number)
{
    if (number.data.empty())
    {
        return os << 0;
    }
    BigInt temp = number;
    string decString;
    while (!(temp == BigInt(0)))
    {
        BigInt q;         // Thương
        uint64_t rem = 0; // Số dư
        q.data.resize(temp.data.size());

        /*
            Giống như chia tay, ta xét từ hàng lớn nhất chia trước
            1. Ví dụ với hệ cơ số 10: BASE = 10
            2. rem << 32 --> Số dư từ bước trước dịch trái 32 bit (Nghĩa là nhân với BASE)
        */
        for (int i = (int)temp.data.size() - 1; i >= 0; --i)
        {
            uint64_t cur = (rem << 32) + temp.data[i];
            q.data[i] = (uint32_t)(cur / 10);
            rem = cur % 10;
        }

        /*
            q = [2, 5]
            decString = "3"
            temp = [2, 5]
            Tiếp tục thực hiện lại
        */
        q.trim();
        decString.push_back('0' + rem);
        temp = q;
    }
    /*
        Sau vòng lặp: decString = "325"
        Reverse --> decString = "523"
    */
    reverse(decString.begin(), decString.end());
    os << decString;
    return os;
}

// Toán tử mod_exp
/*
    @param base (Cơ số)
    @param exp (Số mũ)
    @param mod (Số mod)
    @return base^exp % mod
    @logic
    1. Ý tưởng chính: Phương pháp bình phương và nhân
    2. Nếu exp = 13 = 1101₂ = 2³ + 2² + 2⁰ = 8 + 4 + 1
    --> base^13 = base^(8+4+1) = base^8 × base^4 × base^1
    3. Áp dụng thuật toán Barrett Mod để tối ưu hơn
*/
BigInt BigInt::modular_exponentiation(BigInt base, BigInt exp, const BigInt &mod)
{
    BigInt result(1);
    base = base % mod; // Tính chất: (a % m)^n % m = a^n % m
    while (!(exp == BigInt(0)))
    {
        // Xét exp % 2 == 1 (Kiểm tra bit cuối nếu là 1 thì là số lẻ)
        // Nếu exp lẻ: base^exp = base × base^(exp-1)
        if (!exp.data.empty() && (exp.data[0] & 1))
        {
            result = barrett_mod(result * base, mod);
        }
        // Chuẩn bị base cho bước tiếp theo
        /*
        Ví dụ:
            base = 2
            base = 2^2 = 4
            base = 4^2 = 16
            base = 16^2 = 256
        */
        base = barrett_mod(base * base, mod);
        uint64_t carry = 0;
        for (int i = exp.data.size() - 1; i >= 0; i--)
        {
            uint64_t cur = (carry << 32) + exp.data[i];
            exp.data[i] = (uint32_t)(cur / 2);
            carry = cur % 2;
        }
        exp.trim();
    }

    return result;
}

/*
    @logic

*/
BigInt BigInt::random_bits(int bits)
{
    BigInt result(0);
    int blocks = (bits + 31) / 32;
    mt19937_64 rng((unsigned)time(nullptr));
    result.data.resize(blocks);
    for (int i = 0; i < blocks; i++)
        result.data[i] = (uint32_t)(rng() & 0xFFFFFFFFULL);
    int extra_bits = blocks * 32 - bits;
    if (extra_bits && !result.data.empty())
        result.data.back() &= (0xFFFFFFFFu >> extra_bits);
    if (!result.data.empty())
    {
        result.data.back() |= (1U << (31 - (extra_bits ? extra_bits : 0)));
        result.data[0] |= 1;
    }
    result.trim();
    return result;
}

// Thuật toán Miller-Rabin - kiểm tra số nguyên tố
/*
    @logic
*/
bool BigInt::is_prime_by_Miller_Rabin(const BigInt &n, int iterations)
{
    if (n == BigInt(2) || n == BigInt(3))
        return true;
    if (n < BigInt(2))
        return false;
    if (n.data.empty())
        return false;
    if ((n.data[0] & 1) == 0)
        return false;
    BigInt d = n - BigInt(1);
    int s = 0;
    while (!d.data.empty() && (d.data[0] & 1) == 0)
    {
        uint64_t carry = 0;
        for (int i = (int)d.data.size() - 1; i >= 0; --i)
        {
            uint64_t cur = (carry << 32) + d.data[i];
            d.data[i] = (uint32_t)(cur >> 1);
            carry = cur & 1;
        }
        d.trim();
        s++;
    }
    mt19937_64 rng((unsigned)time(nullptr));
    uniform_int_distribution<uint64_t> dist;
    for (int i = 0; i < iterations; i++)
    {
        BigInt a = BigInt(dist(rng)) % (n - BigInt(4)) + BigInt(2);
        BigInt x = modular_exponentiation(a, d, n);
        if (x == BigInt(1) || x == n - BigInt(1))
            continue;
        bool cont = false;
        for (int r = 0; r < s - 1; r++)
        {
            x = modular_exponentiation(x, BigInt(2), n);
            if (x == n - BigInt(1))
            {
                cont = true;
                break;
            }
        }
        if (cont)
            continue;
        return false;
    }
    return true;
}

// Hàm tạo số nguyên tố
/*
    @logic
    1. Tạo 1 số nguyên tố từ hàm random_bits
    2. Kiểm tra lại xem tính chính xác của số nguyên tố p
*/
BigInt BigInt::generate_prime(int bits)
{
    while (true)
    {
        BigInt p = random_bits(bits);
        if (is_prime_by_Miller_Rabin(p))
            return p;
    }
}

// Hàm tạo số nguyên tố an toàn
/*
    @logic
    1. Với số nguyên tố p được tạo ra
    2. Kiểm ra số p - 1 / 2 có phải là 1 số nguyên tố không bằng Miller-Rabin
*/
BigInt BigInt::generate_safe_prime(int bits)
{
    int q_bits = bits - 1;
    while (true)
    {
        BigInt q = BigInt::generate_prime(q_bits);
        BigInt p = q * 2 + 1;
        if (BigInt::is_prime_by_Miller_Rabin(p, 7))
        {
            return p;
        }
    }
};

// Hàm sinh khóa riêng trong khoảng [2, p−2]
BigInt BigInt::generate_private_key(BigInt p)
{
    if (p < BigInt(5))
    {
        cout << "p khong hop le [!]" << endl;
        return BigInt(0);
    }

    BigInt key;
    size_t num_blocks = p.data.size();

    // Tạo đủ số block cho data
    key.data.resize(num_blocks);

    // Tạo seed
    srand(static_cast<unsigned>(time(nullptr)));
    // Tạo random giá trị cho từng block
    for (size_t i = 0; i < num_blocks; i++)
    {
        key.data[i] = ((uint32_t)rand() << 16) ^ (uint32_t)rand(); // 32-bit random
    }

    // Đảm bảo giá trị từ 2, p-2
    key = (key % BigInt(p - 3)) + BigInt(2);

    key.trim();

    return key;
}