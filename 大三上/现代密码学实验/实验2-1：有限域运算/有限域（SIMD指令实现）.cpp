#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdint.h>
#include <immintrin.h>

const uint64_t f[3] = {0x0000000000002007, 0x0000000000000000, 0x0000000000000008};

#ifdef ONLINE_JUDGE
#define in cin
#define out cout
#else
std::ifstream input("input2.bin", std::ios::binary);
std::ofstream output("output.bin", std::ios::binary);
#define in input
#define out output
#endif

#ifdef __GNUC__
#pragma GCC target("pclmul,sse2")
#endif

using namespace std;

class F
{
public:
    uint64_t val[5];
    F(uint64_t *num, int n)
    {
        for (int i = 0; i < n; i++)
        {
            val[i] = num[i];
        }
        for (int i = n; i < 5; i++)
        {
            val[i] = 0;
        }
    }
    F mod(uint64_t const *a)
    {
        uint64_t e[5];
        uint64_t res[5] = {0, 0, 0, 0, 0};
        for (int i = 0; i < 5; i++)
        {
            e[i] = a[i];
        }

        for (int i = 4; i >= 3; i--)
        {
            uint64_t t = e[i];
            e[i - 3] = e[i - 3] ^ (t << 61) ^ (t << 62) ^ (t << 63);
            e[i - 2] = e[i - 2] ^ (t << 10) ^ (t >> 1) ^ (t >> 2) ^ (t >> 3);
            e[i - 1] = e[i - 1] ^ (t >> 54);
        }

        uint64_t t = e[2] >> 3;
        res[0] = e[0] ^ (t << 13) ^ t ^ (t << 1) ^ (t << 2);
        res[1] = e[1] ^ (t >> 51);
        res[2] = e[2] & 0x7;

        return F(res, 5);
    }
    F operator+(const F &other) const
    {
        uint64_t res[3];
        for (int i = 0; i < 3; i++)
        {
            res[i] = this->val[i] ^ other.val[i];
        }
        return F(res, 3);
    }
    F operator*(const F &other)
    {
        uint64_t a[3], b[3];
        for (int i = 0; i < 3; i++)
        {
            a[i] = this->val[i];
            b[i] = other.val[i];
        }

        __m128i A0 = _mm_set_epi64x(0, a[0]);
        __m128i A1 = _mm_set_epi64x(0, a[1]);
        __m128i A2 = _mm_set_epi64x(0, a[2]);
        __m128i B0 = _mm_set_epi64x(0, b[0]);
        __m128i B1 = _mm_set_epi64x(0, b[1]);
        __m128i B2 = _mm_set_epi64x(0, b[2]);

        __m128i S0 = _mm_xor_si128(A0, A1);
        __m128i S1 = _mm_xor_si128(A1, A2);
        __m128i S2 = _mm_xor_si128(A0, A2);
        __m128i S3 = _mm_xor_si128(B0, B1);
        __m128i S4 = _mm_xor_si128(B1, B2);
        __m128i S5 = _mm_xor_si128(B0, B2);

        __m128i P0 = _mm_clmulepi64_si128(A0, B0, 0x00);
        __m128i P1 = _mm_clmulepi64_si128(A1, B1, 0x00);
        __m128i P2 = _mm_clmulepi64_si128(A2, B2, 0x00);
        __m128i P3 = _mm_xor_si128(_mm_clmulepi64_si128(S0, S3, 0x00), _mm_xor_si128(P0, P1));
        __m128i P4 = _mm_xor_si128(_mm_clmulepi64_si128(S2, S5, 0x00), _mm_xor_si128(P0, P2));
        __m128i P5 = _mm_xor_si128(_mm_clmulepi64_si128(S1, S4, 0x00), _mm_xor_si128(P1, P2));

        uint64_t res[5] = {0};
        res[0] = _mm_extract_epi64(P0, 0);
        res[1] = _mm_extract_epi64(P3, 0) ^ _mm_extract_epi64(P0, 1);
        res[2] = _mm_extract_epi64(_mm_xor_si128(P1, P4), 0) ^ _mm_extract_epi64(P3, 1);
        res[3] = _mm_extract_epi64(P5, 0) ^ _mm_extract_epi64(_mm_xor_si128(P1, P4), 1);
        res[4] = _mm_extract_epi64(P2, 0) ^ _mm_extract_epi64(P5, 1);
        // res[5] = _mm_extract_epi64(P2, 1);

        return mod(res);
    }
    F inv()
    {
        uint64_t u[5] = {0}, v[5] = {0};
        for (int i = 0; i < 3; i++)
        {
            u[i] = this->val[i];
            v[i] = f[i];
        }

        uint64_t g1[5] = {1, 0, 0, 0, 0};
        uint64_t g2[5] = {0, 0, 0, 0, 0};
        while (degree(u, 5))
        {
            int d = degree(u, 5) - degree(v, 5);
            if (d < 0)
            {
                swap(u, v);
                swap(g1, g2);
                d = -d;
            }

            F tem = left_shift(v, d);
            for (int i = 0; i < 5; i++)
            {
                u[i] ^= tem.val[i];
            }

            tem = left_shift(g2, d);
            for (int i = 0; i < 5; i++)
            {
                g1[i] ^= tem.val[i];
            }
        }
        F tem = mod(g1);
        for (int i = 0; i < 5; i++)
        {
            g1[i] = tem.val[i];
        }

        return F(g1, 5);
    }
    friend ostream &operator<<(ostream &os, const F &f)
    {
        for (int i = 0; i < 3; i++)
            os.write((char *)&(f.val[i]), sizeof(f.val[i]));

        return os;
    }
    int degree(uint64_t const *a, int n)
    {
        for (int i = 0; i < n; i++)
        {
            uint64_t tem = 0x8000000000000000;
            for (int j = 0; j < 64; j++)
            {
                if (a[n - 1 - i] & tem)
                {
                    return (n - i) * 64 - j - 1;
                }
                tem >>= 1;
            }
        }
        return -1;
    }
    F left_shift(uint64_t const *a, int d)
    {
        uint64_t res[5];

        if (d % 64 == 0)
        {
            int tem = d / 64;
            for (int i = 0; i < tem; i++)
            {
                res[i] = 0;
            }
            for (int i = tem; i < 5; i++)
            {
                res[i] = a[i - tem];
            }

            return F(res, 5);
        }
        else
        {
            if (d < 64)
            {
                for (int i = 4; i > 0; --i)
                {
                    res[i] = (a[i] << d) | (a[i - 1] >> (64 - d));
                }
                res[0] = a[0] << d;
            }
            else if (d < 128)
            {
                d -= 64;
                for (int i = 4; i > 1; --i)
                {
                    res[i] = (a[i - 1] << d) | (a[i - 2] >> (64 - d));
                }
                res[1] = a[0] << d;
                res[0] = 0;
            }
            else if (d < 192)
            {
                d -= 128;
                for (int i = 4; i > 2; --i)
                {
                    res[i] = (a[i - 2] << d) | (a[i - 3] >> (64 - d));
                }
                res[2] = a[0] << d;
                res[1] = 0;
                res[0] = 0;
            }

            return F(res, 5);
        }
    }
    void print()
    {
        for (int i = 0; i < 5; i++)
        {
            uint64_t tem = 1;
            for (int j = 0; j < 63; j++)
            {
                if (val[i] & tem)
                {
                    printf("x^%d+", i * 64 + j);
                }
                tem <<= 1;
            }
        }
        printf("\n");
    }
};

int main(void)
{
    uint32_t count;
    in.read((char *)&count, sizeof(count));
    while (count--)
    {
        uint8_t type;
        uint64_t num[2][3];
        in.read((char *)&type, sizeof(type));
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 3; j++)
                in.read((char *)&num[i][j], sizeof(num[i][j]));

        F f1 = F(num[0], 3);
        F f2 = F(num[1], 3);

        if (type == 0x00)
        {
            out << f1 + f2;
        }
        else if (type == 0x01)
        {
            out << f1 * f2;
        }
        else if (type == 0x02)
        {
            out << f1 * f1;
        }
        else if (type == 0x03)
        {
            out << f1.inv();
        }
    }
}