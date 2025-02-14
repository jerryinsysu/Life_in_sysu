#include <immintrin.h>
#include <iostream>
#include <fstream>
#include <bitset>
#include <stdint.h>
#include <iomanip>
#include <vector>
#include <cstring>
using namespace std;
#ifdef ONLINE_JUDGE
#define in cin
#define out cout
#else
std::ifstream input("D:\\Cryptolab\\final\\test8.bin", std::ios::binary);
std::ofstream output("output.txt");
#define in input
#define out output
#endif

#define SMP 8
// ��������
uint32_t P[SMP] = {0}; // ģ��
uint32_t g[SMP] = {0}; // ����Ԫ
uint32_t s[SMP] = {0}; // ��ʼ����
uint32_t len = 0;      // ���ɵ����������
const uint64_t BASE = 0x100000000;
uint32_t ONE[SMP] = {1};
uint32_t pow2[32] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,
                     16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152,
                     4194304, 8388608, 16777216, 33554432, 67108864, 134217728,
                     268435456, 536870912, 1073741824, 2147483648};

// �����������
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
// �Ƚ�������С
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

void mod_mul_improve(uint32_t res[SMP], uint32_t a[SMP], uint32_t b[SMP])
{
    uint32_t temp[SMP] = {0};
    mul(temp, a, b);
    mod(res, temp);
}

void mod_pow_improve(uint32_t res[SMP], uint32_t a[SMP], uint32_t b[SMP])
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
                mod_mul_improve(temp, temp, base);

            mod_mul_improve(base, base, base);
        }
    }
    for (int i = 0; i < SMP; i++)
        res[i] = temp[i];
}

// ����һ������������飬Ȼ�������ɵ����������
uint32_t TWO[SMP] = {2};
void seed(uint32_t length)
{
    // �������
    uint8_t *result = new uint8_t[length](); // ��ʼ��Ϊ0
    // �ж���
    uint32_t judge[SMP] = {0};
    div(judge, P, TWO); // ���� judge = P / 2
    // �ݴ���
    uint32_t m[SMP] = {0};
    uint32_t tmp[SMP] = {0};
    uint32_t count_0 = 0;
    uint32_t count_1 = 0;
    for (uint32_t i = 0; i < length * 8; i++) // ����ÿһ�� bit
    {
        mod_pow_improve(tmp, g, s); // ���� tmp = g^s % P
        uint8_t random_bit = 0;
        if (bigger(judge, tmp)) // �Ƚ� tmp �� judge���ж���� bit
        {
            random_bit = 0; // �����Ϊ 0
            count_0++;
        }
        else
        {
            random_bit = 1; // �����Ϊ 1
            count_1++;
        }

        // д�� result ����Ķ�Ӧλ��
        uint32_t byte_index = (i / 8);                      // �ҵ���ǰ bit �������ֽ�
        uint32_t bit_position = i % 8;                      // ��ǰ bit ���ֽ��е�λ��
        result[byte_index] |= (random_bit << bit_position); // ���ö�Ӧλ

        memcpy(s, tmp, SMP * sizeof(uint32_t)); // ��������
    }

    // ��ӡ�������
    // const uint32_t bytes_per_row = 16; // ÿ����ʾ���ֽ���
    // for (uint32_t i = 0; i < length; i++)
    // {
    //     // �����ǰ�ֽڵ�ʮ������ֵ
    //     printf("%02X ", result[i]);

    //     // ÿ 16 ���ֽڻ�һ��
    //     if ((i + 1) % bytes_per_row == 0)
    //     {
    //         printf("\n");
    //     }
    // }

    // // ������һ�в��� 16 ���ֽڣ��ֶ�����
    // if (length % bytes_per_row != 0)
    // {
    //     printf("\n");
    // }
    // // ��ӡ�������
    // printf("0: %d\n", count_0);
    // printf("1: %d\n", count_1);
    out.write((char *)result, length);
    out.write((char *)&count_0, sizeof(uint32_t));
    out.write((char *)&count_1, sizeof(uint32_t));
    // ������̬�ڴ�
    delete[] result;
}

int main()
{
    // ���벿��
    in.read((char *)P, 4 * sizeof(uint32_t));
    in.read((char *)g, 4 * sizeof(uint32_t));
    in.read((char *)s, 4 * sizeof(uint32_t));
    in.read((char *)&len, 4);
    // �����У�����ģ���̶����Կ����û������Ĵ�������
    seed(len);
}