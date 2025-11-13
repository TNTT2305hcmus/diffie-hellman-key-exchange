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

// Toán tử cộng
/*
    @param other (BigInt cần cộng vào)
    @logic
    1. Áp dụng quy tắc cộng 2 số nguyên thông thường
    2. Cộng từng chữ số theo vị trí tương ứng (cộng thêm carry)
    3. Sử dụng biến uint64_t carry làm số nhớ
    @example
    Giả sử hệ cơ số 10
    Phép cộng: 78 + 345 = 423
    Số 1: [8, 7]     --> 78
    Số 2: [5, 4, 3]  --> 345

    i=0: sum = 0 + 8 + 5 = 13
        data[0] = 3, carry = 1

    i=1: sum = 1 + 7 + 4 = 12
        digits[1] = 2, carry = 1

    i=2: sum = 1 + 0 + 3 = 4
        digits[2] = 4, carry = 0

    Kết quả: [3, 2, 4] → 423
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
    return result;
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
    @example
    Giả sử hệ cơ số 10: BASE = 10
    Phép trừ 523 - 178:
    Số 1: [3, 2, 5]  → 523
    Số 2: [8, 7, 1]  → 178
    Trừ:  523 - 178 = 345

    i=0: diff = 3 - 8 + 0 = -5
        diff < 0 → diff = -5 + 10 = 5, carry = -1
        data[0] = 5

    i=1: diff = 2 - 7 + (-1) = -6
        diff < 0 → diff = -6 + 10 = 4, carry = -1
        data[1] = 4

    i=2: diff = 5 - 1 + (-1) = 3
        diff >= 0 → carry = 0
        data[2] = 3

    Kết quả: [5, 4, 3] → 345
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
    result.trim(); // Xóa các chữ số 0 vô nghĩa có thể có
    return result;
}

// Toán tử nhân
/*
    @param other (Giá trị BigInt nhân vào)
    @logic
    1. Đặt size của result là tổng size 2 số nhân vào
    2. Thực hiện phép nhân 2 số như phép nhân tay 2 số nguyên
    3. Ý tưởng chính
    --> Với mỗi chữ số của data, nhân nó với other.data. Dịch carry sang trái 32 bit mỗi khi nhân xong
    @example
      123  (data)
    ×  45  (other)
    -----
      615  (123 × 5)
    4920   (123 × 4, dịch trái 1 vị trí)
    -----
    5535
*/
BigInt BigInt::operator*(const BigInt &other) const
{
    BigInt result;
    result.data.assign(data.size() + other.data.size(), 0);

    for (int i = 0; i < data.size(); i++)
    {
        uint64_t carry = 0;
        /*
            cur = Kết quả cũ + (chữ số i * chữ số j) + carry
            Ta cần xét j và other.data.size(). Nếu j lớn hơn có nghĩa là chữ số i * 0
        */
        for (int j = 0; j < other.data.size(); j++)
        {
            uint64_t cur = result.data[i + j] + (uint64_t)data[i] * (j < other.data.size() ? other.data[j] : 0) + carry;
            result.data[i + j] = (uint32_t)(cur & 0xFFFFFFFF);
            carry = cur >> 32;
        }
        if (carry > 0)
        {
            result.data[i + other.data.size()] += carry;
        }
    }

    result.trim();
    return result;
}

// Toán tử mod
/*
    @param mod (Giá trị cần mod)
    @logic
    1. Ý tưởng tổng quát: a % m = a - m × ⌊a/m⌋
*/
BigInt BigInt::operator%(const BigInt &mod) const
{
    BigInt cur = *this;

    // Lặp cho đến khi cur < mod
    // cur >= mod : Vì không có toán tử >= và ! nên tối ưu
    while (cur < mod == false)
    {
        BigInt power = mod;

        // Tìm lũy thừa của 2 lớn nhất: mod * 2^k <= cur
        BigInt nextPower = power * 2; // power * 2
        while (cur < nextPower == false)
        {
            power = nextPower;
            nextPower = power * 2;
        }

        cur = cur - power;
    }

    return cur;
}

// Xuất
/*
    @param os (luồng xuất)
    @param number (Số BigInt cần xuất ra màn hình)
    @logic
    1. Nếu number.data rỗng --> Số 0
    2. Ý tưởng chính: Chia liên tục cho 10 và lấy phần dư làm chữ số
    3. Chuyển chữ số này sang chuỗi decString để xuất ra màn hình
    4. Ví dụ: số 12345
    12345 ÷ 10 = 1234 dư 5  → chữ số '5'
    1234 ÷ 10 =  123 dư 4  → chữ số '4'
    123 ÷ 10 =   12 dư 3  → chữ số '3'
    12 ÷ 10 =    1 dư 2  → chữ số '2'
    1 ÷ 10 =    0 dư 1  → chữ số '1'
    Ghép ngược lại: "12345"
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
            3. Xét phép chia 523 cho 10:
            Thương: [3, 2, 5]  → 523

            i=2: cur = 0×10 + 5 = 5
            q[2] = 5÷10 = 0
            rem = 5%10 = 5

            i=1: cur = 5×10 + 2 = 52
                q[1] = 52÷10 = 5
                rem = 52%10 = 2

            i=0: cur = 2×10 + 3 = 23
                q[0] = 23÷10 = 2
                rem = 23%10 = 3

            Thương q = [2, 5, 0] = 52
            Dư rem = 3
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
*/
BigInt BigInt::modular_exponentiation(BigInt base, BigInt exp, const BigInt &mod)
{
    BigInt result(1);
    base = base % mod; // Tính chất: (a % m)^n % m = a^n % m
    while (!(exp == BigInt(0)))
    {
        // Xét exp % 2 == 1 (Kiểm tra bit cuối nếu là 1 thì là số lẻ)
        // Nếu exp lẻ: base^exp = base × base^(exp-1)
        if (exp.data[0] & 1)
        {
            result = (result * base) % mod;
        }
        // Chuẩn bị base cho bước tiếp theo
        /*
        Ví dụ:
            base = 2
            base = 2^2 = 4
            base = 4^2 = 16
            base = 16^2 = 256
        */
        base = (base * base) % mod;

        /*
            Mục đích: exp = exp / 2
            Giống như chia tay
            Ví dụ: exp = [3, 2, 1] --> Số 123
            Phép chia: 123 / 2 (hệ cơ số 10)

            i=2: cur = 0×10 + 1 = 1
            exp[2] = 1÷2 = 0, carry = 1

            i=1: cur = 1×10 + 2 = 12
            exp[1] = 12÷2 = 6, carry = 0

            i=0: cur = 0×10 + 3 = 3
                exp[0] = 3÷2 = 1, carry = 1

            exp = [1, 6, 0] → trim → [1, 6] = 61 ✓
            123 ÷ 2 = 61 (dư 1)
        */
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

// Hàm sinh khóa riêng trong khoảng [2, p−2]
BigInt BigInt::generate_private_key(BigInt p)
{
    if (p < BigInt(5))
    {
        cout << "p khong hop le [!]" << endl;
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