#include <iostream>
#include <fstream>
#include <bitset>
#include <stdint.h>
#include <iomanip>
using namespace std;

#ifdef ONLINE_JUDGE
#define in cin
#define out cout
#else
std::ifstream input("D:/Cryptolab/dump.bin", std::ios::binary);
std::ofstream output("output.txt");
#define in input
#define out output
#endif

const int M = 131;          // 定义多项式的位数
const int m_bitset = 2 * M; // 每个多项式是3个64位组成，总共192位

typedef std::bitset<m_bitset> gf; // 用bitset来表示多项式

gf f;
void init()
{
    f.set(131);
    f.set(13);
    f.set(2);
    f.set(1);
    f.set(0);
    // cout << f;
}

// 将 uint64_t 数组转换为 bitset
gf to_bitset(uint64_t data[3])
{
    gf result;
    // 将每个 uint64_t 的64位填入 bitset 的相应位置
    for (int i = 0; i < 3; ++i)
    {
        std::bitset<64> part(data[i]); // 每个64位部分
        for (int j = 0; j < 64; ++j)
        {
            result[i * 64 + j] = part[j]; // 将64位数填入bitset的相应位置
        }
    }
    return result;
}

// 将 Poly 转换回 uint64_t 数组
void to_uint64_array(const gf &poly, uint64_t data[3])
{
    for (int i = 0; i < 3; ++i)
    {
        data[i] = 0; // 初始化为0
        for (int j = 0; j < 64; ++j)
        {
            data[i] |= (static_cast<uint64_t>(poly[i * 64 + j]) << j);
        }
    }
}

void gf_print(const uint64_t data[3])
{
    // 先将数据按小端顺序存储到一个字节数组中
    // uint8_t byte_array[24]; // 3 * 8 字节
    // for (int i = 0; i < 3; ++i)
    // {
    //     for (int j = 0; j < 8; ++j)
    //     {
    //         byte_array[i * 8 + j] = (data[i] >> (j * 8)) & 0xFF; // 小端存储
    //     }
    // }

    // // 打印字节数组
    // for (int i = 0; i < 24; ++i)
    // {
    //     std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte_array[i]);
    //     if ((i + 1) % 8 == 0)
    //     {
    //         std::cout << std::endl; // 每8个字节换行
    //     }
    //     else
    //     {
    //         std::cout << " "; // 字节之间添加空格
    //     }
    // }
    for (int i = 0; i < 3; i++)
    {
        out.write((char *)&(data[i]), sizeof(data[i]));
    }
}

gf gf_add(const gf &a, const gf &b)
{
    return a ^ b; // 对两个位数相同的bitset执行按位异或
}

gf gf_mod(gf a) // 取模操作
{
    gf r;
    r.set(13);
    r.set(2);
    r.set(1);
    r.set(0);
    for (int i = 2 * M - 1; i >= M; i--) // 将多余的部分都约分
    {
        if (a.test(i)) // 判断此时的位是否为1
        {
            a.reset(i);
            gf tmp = r << (i - M);
            a = gf_add(a, tmp);
        }
    }
    return a;
}

gf mul(gf a, const gf &b) // 乘法
{
    gf ans(0);                         // 初始化结果为 0
    for (int i = 0; i < b.size(); i++) // 遍历 b 的每一位
    {
        if (b[i])
            ans ^= a; // 这个操作可以用加法替代，但是直接使用感觉会更加明了

        a <<= 1;       // 移位操作是一定要执行的
        if (a.test(M)) // 如果左移后，a 的最高位（第 M 位）为 1
            a ^= f;    // 用模多项式 f 进行约减
    }
    return gf_mod(ans);
}

gf square(const gf &a) // 平方的结果是将多项式的每个项的指数加倍
{
    gf b;
    for (int i = 0; i < M; i++)
        b[i * 2] = a[i];
    return gf_mod(b);
}

// 求逆操作之一
gf Fer(const gf &x)
{
    gf y(x);
    for (int i = 1; i <= M - 2; i++)
    {
        gf z = square(y); // 只需要构造一个2^m-1即可
        y = mul(z, x);
    }
    y = square(y);
    return y;
}

int degree(const gf &a)
{
    for (int i = 2 * M - 1; i >= 0; i--)
        if (a[i] || i == 0)
            return i;
    return -1; // 确保函数总是返回一个值
}

gf Euc(const gf &a, const bitset<M + 1> &p)
{
    gf b;                     // 用于存储逆元
    gf c;                     // 辅助多项式
    gf u = a;                 // 将 a 赋值给 u
    gf v = gf(p.to_string()); // 将模多项式 p 赋值给 v
    b[0] = 1;                 // 初始化 b 为 1，表示常数多项式
    int degU, degV;

    // 计算次数并且循环直至 u 的次数为零
    while ((degU = degree(u)) > 0)
    {
        degV = degree(v);

        // 保证 degU >= degV
        if (degU < degV)
        {
            std::swap(u, v);
            std::swap(b, c);
            std::swap(degU, degV); // 同步更新次数
        }

        // j = degU - degV
        int j = degU - degV;

        // 使用异或进行多项式减法
        u ^= (v << j);
        b ^= (c << j);
    }

    return gf_mod(b); // 返回经过模操作后的逆元
}

// 计算多项式的平方
void gf_pow2(gf &result, const gf &a)
{
    result = square(a);
}

// 计算多项式的乘积
void gf_mul(gf &result, const gf &a, const gf &b)
{
    result = mul(a, b);
}

// 使用费马小定理求逆元
// 使用费马小定理求逆元
void gf_inverse(gf &result, const gf input)
{
    // 定义一个辅助函数来进行多次平方操作
    auto multiple_square = [](gf &output, const gf &value, int times)
    {
        output = value;
        for (int i = 0; i < times; i++)
        {
            gf_pow2(output, output);
        }
    };

    // step1 = input^3
    gf_pow2(result, input);
    gf_mul(result, result, input);
    gf step1 = result;

    // step2 = step1^5
    multiple_square(result, result, 2);
    gf_mul(result, result, step1);
    gf step2 = result;

    // step3 = step2^17
    multiple_square(result, result, 4);
    gf_mul(result, result, step2);
    gf step3 = result;

    // step4 = step3^257
    multiple_square(result, result, 8);
    gf_mul(result, result, step3);
    gf step4 = result;

    // step5 = step4^65537
    multiple_square(result, result, 16);
    gf_mul(result, result, step4);
    gf step5 = result;

    // step6 = step5^(2^32 + 1)
    multiple_square(result, result, 32);
    gf_mul(result, result, step5);
    gf_pow2(result, result);
    gf_mul(result, result, input);
    gf step6 = result;

    // step7 = step6^(2^65 + 1)
    multiple_square(result, result, 65);
    gf_mul(result, result, step6);
    gf_pow2(result, result);

    return;
}
int main()
{
    uint32_t times; // 需要进行的运算数量
    in.read((char *)&times, sizeof(times));

    bitset<M + 1> p("111");
    p[131] = 1;
    p[13] = 1;
    for (int i = 0; i < times; i++)
    {
        uint8_t type; // 需要进行的运算类型
        in.read((char *)&type, sizeof(type));
        uint64_t data[2][3] = {0};

        // 读取二进制数据
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                in.read((char *)&data[i][j], sizeof(data[i][j]));
            }
        }

        // 将第一个多项式转换为bitset并输出
        gf poly0 = to_bitset(data[0]);
        // std::cout << "Poly 0: " << poly0 << std::endl;

        // 将第二个多项式转换为bitset并输出
        gf poly1 = to_bitset(data[1]);
        // std::cout << "Poly 1: " << poly1 << std::endl;

        // 输出多项式的二进制表示
        // std::cout << "Polynomial f(x) in binary: " << f << std::endl;
        gf res;
        uint64_t converted_data[3] = {0};
        switch (type)
        {
        case 0: // Example case for addition
            res = gf_add(poly0, poly1);
            to_uint64_array(res, converted_data);
            gf_print(converted_data);
            break;
        case 1: // Example case for multiplication
            res = mul(poly0, poly1);
            to_uint64_array(res, converted_data);
            gf_print(converted_data);
            break;
        case 2:
            res = square(poly0);
            to_uint64_array(res, converted_data);
            gf_print(converted_data);
            break;
        case 3: // 新增的逆元计算
            gf_inverse(res, poly0);
            to_uint64_array(res, converted_data);
            gf_print(converted_data);
            break;
        // Add more cases as needed
        default:
            break;
        }
    }
}