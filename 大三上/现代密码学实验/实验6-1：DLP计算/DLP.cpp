#include <string>
#include <stdint.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <chrono>
#include <string.h>
using namespace std;

// 本次实验的关键变量，可以根据实际情况进行修改，使其具有更高的可扩展性
#define SMP 8

// 测试样例
// string p = "809";
// string n = "101";
// string g = "89";
// string y = "618";

// string p = "5041259";
// string n = "5041258";
// string g = "2";
// string y = "424242";

// string p = "1145110751406218783309";
// string n = "23274244228";
// string g = "588575730691984005381";
// string y = "231912707483127123412";

string p;
string n;
string g;
string y;

// 变量区域
uint32_t P[SMP] = {0};  // 模数
uint32_t dp[SMP] = {0}; // 模数
uint32_t N[SMP] = {0};  // 生成元阶数
uint32_t G[SMP] = {0};  // 生成元
uint32_t Y[SMP] = {0};  // 目标值
uint32_t s[SMP] = {3};  // 选择
uint32_t t[SMP] = {2};  // 乘法
uint32_t R[SMP] = {0};
uint32_t ONE[SMP] = {1};
int R_bits = 0;
int P_bits = 0;
uint32_t ZERO[SMP] = {0};
uint32_t P_[SMP] = {0};
uint32_t R2[SMP] = {0};
const uint64_t BASE = 0x100000000;
//----------------------------------------------------------------------------------------------
uint32_t pow2[32] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,
                     16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152,
                     4194304, 8388608, 16777216, 33554432, 67108864, 134217728,
                     268435456, 536870912, 1073741824, 2147483648};

int MSB(const uint32_t a[SMP])
{
    for (int i = SMP - 1; i >= 0; i--)
    {
        if (a[i])
            for (int j = 31; j >= 0; j--)
                if (a[i] & pow2[j])
                    return i * 32 + j;
    }
    return 0;
}

// 比较两数大小
bool equal(uint32_t a[SMP], uint32_t b[SMP])
{
    for (int i = 0; i < SMP; i++)
        if (a[i] != b[i])
            return false;
    return true;
}

bool bigger(uint32_t a[SMP], uint32_t b[SMP])
{
    for (int i = SMP - 1; i >= 0; i--)
    {
        if (a[i] > b[i])
            return true;
        else if (a[i] < b[i])
            return false;
    }
    return false;
}

// 输入输出处理
void str2bi(uint32_t res[SMP], string &s)
{
    // 初始化结果数组
    for (int i = 0; i < SMP; i++)
        res[i] = 0;

    int count = 0; // 用于记录当前位的位置
    while (s != "0")
    {
        // 如果当前最低位为 1，则设置对应的二进制位
        if ((s[0] - '0') & 1)
            res[count / 32] += (1U << (count % 32));

        // 将字符串表示的十进制数除以 2
        int carry = 0;
        for (int i = s.length() - 1; i >= 0; i--)
        {
            int x = s[i] - '0';
            s[i] = (x + carry * 10) / 2 + '0';
            carry = x % 2;
        }

        // 移除字符串右侧的多余零
        size_t end = s.find_last_not_of('0');
        if (end != string::npos)
            s.erase(end + 1);
        else
            s = "0";

        count++; // 更新二进制位计数
    }
}

void str_mul(string &s)
{
    int carry = 0;
    for (std::string::size_type i = 0; i < s.length(); i++)
    {
        int x = s[i] - '0';
        s[i] = (x * 2 + carry) % 10 + '0';
        carry = (x * 2 + carry) / 10;
    }

    if (carry)
        s = s + "1";
}

void str_add(string &s)
{
    int carry = 1;
    for (std::string::size_type i = 0; i < s.length(); i++)
    {
        int x = s[i] - '0';
        s[i] = (x + carry) % 10 + '0';
        carry = (x + carry) / 10;
    }
    if (carry)
        s = s + "1";
}

string bi2str(uint32_t res[SMP])
{
    string s = "0";
    for (int i = SMP - 1; i >= 0; i--)
    {
        for (int j = 31; j >= 0; j--)
        {
            str_mul(s);
            if (res[i] & pow2[j])
                str_add(s);
        }
    }
    reverse(s.begin(), s.end());

    return s;
}

void add(uint32_t res[SMP], uint32_t a[SMP], uint32_t b[SMP])
{
    uint64_t carry = 0;
    uint32_t temp[SMP] = {0};
    for (int i = 0; i < SMP; i++)
    {
        uint64_t sum = carry + a[i] + b[i];
        temp[i] = sum & 0xffffffff;
        carry = sum >> 32;
    }

    for (int i = 0; i < SMP; i++)
        res[i] = temp[i];
}

void sub(uint32_t res[SMP], uint32_t a[SMP], uint32_t b[SMP])
{
    uint64_t borrow = 0;
    uint32_t temp[SMP] = {0};
    for (int i = 0; i < SMP; i++)
    {
        uint64_t temp1 = b[i] + borrow;
        if (a[i] < temp1)
        {
            temp[i] = BASE + a[i] - temp1;
            borrow = 1;
        }
        else
        {
            temp[i] = a[i] - temp1;
            borrow = 0;
        }
    }

    for (int i = 0; i < SMP; i++)
        res[i] = temp[i];
}

void mul(uint32_t res[SMP], uint32_t a[SMP], uint32_t b[SMP])
{
    uint32_t temp[SMP * 2] = {0};

    for (int i = 0; i < SMP; i++)
    {
        uint64_t carry = 0;
        for (int j = 0; j < SMP; j++)
        {
            uint64_t sum = (uint64_t)a[i] * b[j] + temp[i + j] + carry;
            temp[i + j] = sum & 0xffffffff;
            carry = sum >> 32;
        }
        if (carry)
            temp[i + SMP] += carry;
    }

    for (int i = 0; i < SMP; i++)
        res[i] = temp[i];
}

void div(uint32_t res[SMP], uint32_t a[SMP], uint32_t b[SMP])
{
    uint32_t temp[SMP] = {0};
    int n = MSB(b) / 32;
    int m = MSB(a) / 32 - n;

    uint32_t d[SMP] = {0};
    d[0] = BASE / (b[n] + (uint64_t)1);
    uint32_t u_[SMP + 1] = {0}, v_[SMP] = {0};

    uint64_t carry = 0;
    for (int i = 0; i < SMP; i++)
    {
        uint64_t temp = (uint64_t)a[i] * d[0] + carry;
        u_[i] = temp & 0xffffffff;
        carry = temp >> 32;
    }
    if (carry)
        u_[SMP] = carry;

    mul(v_, b, d);

    int j = m;

    while (j >= 0)
    {
        uint32_t tem[SMP + 1] = {0};
        for (int i = 0; i <= n + 1; i++)
            tem[i] = u_[i + j];
        uint64_t tem2 = (tem[n + 1] * BASE + tem[n]) / v_[n];
        tem2 = min(tem2, BASE - 1);

        uint32_t q_hat[SMP] = {static_cast<uint32_t>(tem2 & 0xffffffff)};
        uint32_t qv[SMP] = {0};
        mul(qv, v_, q_hat);
        while (bigger(qv, tem))
        {
            q_hat[0]--;
            sub(qv, qv, v_);
        }

        sub(tem, tem, qv);
        for (int i = 0; i <= n + 1; i++)
            u_[i + j] = tem[i];

        temp[j] = q_hat[0];

        j--;
    }

    for (int i = 0; i < SMP; i++)
        res[i] = temp[i];
}

void mod(uint32_t res[SMP], uint32_t a[SMP])
{
    uint32_t temp[SMP] = {0};
    div(temp, a, P);
    mul(temp, temp, P);
    sub(res, a, temp);
}

// 这里是为了能够适应本题而修改的内容函数具体的参数，大致与之前的函数没有多少区别

void mod_improve(uint32_t res[SMP], uint32_t a[SMP], uint32_t m[SMP])
{
    uint32_t temp[SMP] = {0};
    div(temp, a, m);
    mul(temp, temp, m);
    sub(res, a, temp);
}

void mod_mul_improve(uint32_t res[SMP], uint32_t a[SMP], uint32_t b[SMP], uint32_t m[SMP])
{
    uint32_t temp[SMP] = {0};
    mul(temp, a, b);
    mod_improve(res, temp, m);
}

void mod_pow_improve(uint32_t res[SMP], uint32_t a[SMP], uint32_t b[SMP], uint32_t m[SMP])
{
    uint32_t temp[SMP] = {1};
    uint32_t base[SMP] = {0};
    for (int i = 0; i < SMP; i++)
        base[i] = a[i];

    for (int i = 0; i < SMP; i++)
    {
        if (bigger(temp, ONE) && b[i] == 0)
            break;
        for (int j = 0; j < 32; j++)
        {
            if (b[i] & pow2[j])
                mod_mul_improve(temp, temp, base, m);

            mod_mul_improve(base, base, base, m);
        }
    }
    for (int i = 0; i < SMP; i++)
        res[i] = temp[i];
}

void mod_add_improve(uint32_t res[SMP], uint32_t a[SMP], uint32_t b[SMP], uint32_t m[SMP])
{
    uint32_t temp[SMP] = {0};
    add(temp, a, b);
    mod_improve(res, temp, m);
}

uint32_t TWO[SMP] = {2};

// 这个是费马小定理来求逆元（由于题目的局限性，该求逆方法只好放弃）
void fermat(uint32_t res[SMP], uint32_t a[SMP], uint32_t m[SMP])
{
    uint32_t temp[SMP];
    sub(temp, m, TWO);
    mod_pow_improve(res, a, temp, m);
}

void uint32ArrayToUint128(const uint32_t n[8], __uint128_t &t)
{
    t = 0; // 初始化 t 为 0
    for (int i = 0; i < 8; ++i)
    {
        // 将数组的每一部分整合到 t 中
        t |= static_cast<__uint128_t>(n[i]) << (i * 32);
    }
}

void uint128ToUint32Array(__uint128_t t, uint32_t n[8])
{
    for (int i = 0; i < 8; ++i)
    {
        n[i] = static_cast<uint32_t>(t & 0xFFFFFFFF); // 提取低 32 位
        t >>= 32;                                     // 右移 32 位
    }
}
// 大数库结束-------------------------------------------------------------------------------------------------

// void f(uint32_t x[SMP], uint32_t a[SMP], uint32_t b[SMP])
// {
//     uint32_t state[SMP] = {0};
//     mod_improve(state, x, s);
//     // 因为最后是mod 3的，所以只需要考虑state[0]的值
//     if (state[0] == 0)
//     {
//         mod_mul_improve(x, x, x, dp);
//         mod_add_improve(a, a, a, N);
//         mod_add_improve(b, b, b, N);
//     }
//     else if (state[0] == 1)
//     {
//         mod_mul_improve(x, x, Y, dp);
//         mod_add_improve(b, b, ONE, N);
//     }
//     else
//     {
//         mod_mul_improve(x, x, G, dp);
//         mod_add_improve(a, a, ONE, N);
//     }
// }

int mod_int_improve(uint32_t a[SMP], int mod)
{
    __uint128_t t = 0;
    for (int i = SMP - 1; i >= 0; i--)
    {
        t = (t * BASE + a[i]) % mod;
    }
    return static_cast<int>(t);
}

void f(uint32_t x[SMP], __uint128_t &a, __uint128_t &b, __uint128_t n_improve)
{
    int temp = mod_int_improve(x, 3);
    uint32_t state[SMP] = {0};
    mod_improve(state, x, s);
    // 因为最后是mod 3的，所以只需要考虑state[0]的值
    if (state[0] == 0)
    {
        mod_mul_improve(x, x, x, dp);
        a = a * 2 % n_improve;
        b = b * 2 % n_improve;
    }
    else if (state[0] == 1)
    {
        mod_mul_improve(x, x, Y, dp);
        b = (b + 1) % n_improve;
    }
    else
    {
        mod_mul_improve(x, x, G, dp);
        a = (a + 1) % n_improve;
    }
}

void inv_exculid_improve(uint32_t a[SMP], uint32_t b[SMP], uint32_t x[SMP], uint32_t y[SMP], uint32_t m[SMP])
{
    bool flag = false;
    for (int i = 0; i < SMP; i++)
        if (b[i])
        {
            flag = true;
            break;
        }

    if (!flag)
    {
        x[0] = 1;
        y[0] = 0;
        return;
    }

    uint32_t temp[SMP] = {0};
    div(temp, a, b);
    mul(temp, b, temp);
    sub(temp, a, temp);

    inv_exculid_improve(b, temp, y, x, m);

    uint32_t temp2[SMP] = {0};
    div(temp2, a, b);
    mul(temp2, temp2, x);

    if (bigger(y, temp2))
    {
        sub(temp2, y, temp2);
        mod_improve(temp2, temp2, m);
    }
    else
    {
        sub(temp2, temp2, y);
        mod_improve(temp2, temp2, m);
        sub(temp2, m, temp2);
    }

    for (int i = 0; i < SMP; i++)
        y[i] = temp2[i];
}

void gcd(uint32_t a[SMP], uint32_t b[SMP], uint32_t res[SMP])
{
    uint32_t r[SMP] = {0}, temp[SMP] = {0};
    memcpy(temp, b, SMP * sizeof(uint32_t)); // t = n
    memcpy(res, a, SMP * sizeof(uint32_t));  // x = t
    while (!equal(temp, ZERO))
    {
        mod_add_improve(r, res, ZERO, temp);
        for (int i = 0; i < SMP; ++i)
            swap(res[i], temp[i]);
        for (int i = 0; i < SMP; ++i)
            swap(temp[i], r[i]);
    }
}

void Pollard(uint32_t result[SMP])
{
    uint32_t x[SMP] = {0}, a[SMP] = {0}, b[SMP] = {0};
    uint32_t X[SMP] = {0}, A[SMP] = {0}, B[SMP] = {0};
    __uint128_t n_improve = 0;
    __uint128_t a_improve = 0;
    __uint128_t b_improve = 0;
    __uint128_t A_improve = 0;
    __uint128_t B_improve = 0;
    uint32ArrayToUint128(N, n_improve);
    x[0] = 1;
    X[0] = 1;
    f(x, a_improve, b_improve, n_improve);
    f(X, A_improve, B_improve, n_improve);
    f(X, A_improve, B_improve, n_improve);
    while (!equal(x, X))
    {
        f(x, a_improve, b_improve, n_improve);
        f(X, A_improve, B_improve, n_improve);
        f(X, A_improve, B_improve, n_improve);
    }
    uint128ToUint32Array(a_improve, a);
    uint128ToUint32Array(b_improve, b);
    uint128ToUint32Array(A_improve, A);
    uint128ToUint32Array(B_improve, B);
    uint32_t tmp_X[SMP] = {0}, tmp_Y[SMP] = {0};
    uint32_t res_A[SMP], res_B[SMP] = {0};
    uint32_t tmp_B[SMP] = {0};
    uint32_t tmp_N[SMP] = {0};
    uint32_t gcd_num[SMP] = {0};
    add(res_B, B, N);
    sub(res_B, res_B, b);
    add(res_A, a, N);
    sub(res_A, res_A, A);
    memcpy(tmp_B, res_B, SMP * sizeof(uint32_t));
    memcpy(tmp_N, N, SMP * sizeof(uint32_t));
    gcd(res_B, tmp_N, gcd_num);
    if (equal(gcd_num, ONE)) // 代表其有逆元素
    {
        uint32_t inv[SMP] = {0};
        inv_exculid_improve(res_B, tmp_N, tmp_X, tmp_Y, N);
        mod_improve(tmp_X, tmp_X, N);
        mod_mul_improve(inv, tmp_X, res_A, N);
        cout << bi2str(inv) << endl;
    }
    else // 此时存在多解
    {
        // 此时tmp_B里面存在着我们循环解的多个结果
        // 这里迭代器的值默认是不会超过2^32的
        int iter = stoi(bi2str(gcd_num));
        // 方程两边全部同时除以公约数tmp_B
        div(res_A, res_A, gcd_num); // a = a / gcd
        div(res_B, res_B, gcd_num); // b = b / gcd
        div(tmp_N, N, gcd_num);     // n = n / gcd
        // 此时，由于已经互素了，所以可以通过上面的方法重新得到一个解
        uint32_t inv[SMP] = {0};
        inv_exculid_improve(res_B, tmp_N, inv, tmp_Y, tmp_N);
        mod_mul_improve(inv, inv, res_A, tmp_N); // 此时的inv是众多解中的一个
        uint32_t one[SMP] = {1};
        uint32_t tmp[SMP] = {0};
        uint32_t tmp_ans[SMP] = {0};
        for (int i = 0; i < iter; i++)
        {
            mod_mul_improve(tmp, tmp, tmp_N, N);
            mod_add_improve(inv, inv, tmp, N);
            //  检测当前值是否是离散对数解
            mod_pow_improve(tmp_ans, G, inv, dp);
            if (equal(tmp_ans, Y))
            {
                cout << bi2str(inv) << endl;
            }
            add(tmp, tmp, one);
        }
    }
}

// 本次实验的主函数
int main(void)
{
    cin >> p >> n >> g >> y;
    //   先将读入的数据变成可供我们处理的数据
    reverse(p.begin(), p.end());
    str2bi(dp, p);

    reverse(n.begin(), n.end());
    str2bi(N, n);

    reverse(g.begin(), g.end());
    str2bi(G, g);

    reverse(y.begin(), y.end());
    str2bi(Y, y);

    uint32_t result[SMP] = {0};
    Pollard(result);
}