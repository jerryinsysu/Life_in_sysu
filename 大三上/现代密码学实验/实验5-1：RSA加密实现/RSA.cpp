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
std::ifstream input("D:\\Cryptolab\\RSA\\test1.bin", std::ios::binary);
std::ofstream output("output.txt");
#define in input
#define out output
#endif

#define DB_len 223     // 数据块长度
uint32_t P[128] = {0}; // 模数

void get_seed(uint8_t seed[32])
{
    uint16_t tmp;
    for (int i = 0; i < 16; ++i)
    {
        _rdrand16_step(&tmp);
        seed[i * 2] = tmp >> 8;
        seed[i * 2 + 1] = tmp & 0xFF;
    }
}

// 以下是hash函数的实现
// 设置初始哈希值，这里也相当于初始化操作
uint32_t H[8] = {
    0x6a09e667,
    0xbb67ae85,
    0x3c6ef372,
    0xa54ff53a,
    0x510e527f,
    0x9b05688c,
    0x1f83d9ab,
    0x5be0cd19};

// 设置初始常数
uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

// 定义相关的逻辑函数
#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))              // 逻辑函数 Ch
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))       // 右旋转
#define Ma(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z))) // 逻辑函数 Ma(两位两位取异或)
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))       // 逻辑函数 EP0
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))       // 逻辑函数 EP1
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))       // 逻辑函数 SIG0
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))     // 逻辑函数 SIG1

// 调用该中间过程，而该过程一定是对512位数据进行处理，所以不需要记录长度变量
void sha256(uint8_t *data, uint32_t tmp[8])
{
    uint32_t W[64] = {0};            // 用于存储扩展后的数据
    uint32_t a, b, c, d, e, f, g, h; // 用于存储中间变量
    uint32_t T1, T2;                 // 用于存储临时变量
    // 初始化中间变量
    a = H[0];
    b = H[1];
    c = H[2];
    d = H[3];
    e = H[4];
    f = H[5];
    g = H[6];
    h = H[7];

    tmp[0] = H[0];
    tmp[1] = H[1];
    tmp[2] = H[2];
    tmp[3] = H[3];
    tmp[4] = H[4];
    tmp[5] = H[5];
    tmp[6] = H[6];
    tmp[7] = H[7];

    // 对数据进行扩展
    for (int i = 0; i < 16; i++)
    {
        W[i] = (data[i * 4] << 24) | (data[i * 4 + 1] << 16) | (data[i * 4 + 2] << 8) | data[i * 4 + 3];
    }
    for (int i = 16; i < 64; i++)
    {
        W[i] = SIG1(W[i - 2]) + W[i - 7] + SIG0(W[i - 15]) + W[i - 16];
    }
    // 进行压缩
    for (int i = 0; i < 64; i++)
    {
        T1 = h + EP1(e) + Ch(e, f, g) + K[i] + W[i];
        T2 = EP0(a) + Ma(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }
    // 更新哈希值
    tmp[0] += a;
    tmp[1] += b;
    tmp[2] += c;
    tmp[3] += d;
    tmp[4] += e;
    tmp[5] += f;
    tmp[6] += g;
    tmp[7] += h;
}

void sha256_DB(uint8_t *data)
{
    uint32_t W[64] = {0};            // 用于存储扩展后的数据
    uint32_t a, b, c, d, e, f, g, h; // 用于存储中间变量
    uint32_t T1, T2;                 // 用于存储临时变量
    // 初始化中间变量
    a = H[0];
    b = H[1];
    c = H[2];
    d = H[3];
    e = H[4];
    f = H[5];
    g = H[6];
    h = H[7];
    // 对数据进行扩展
    for (int i = 0; i < 16; i++)
    {
        W[i] = (data[i * 4] << 24) | (data[i * 4 + 1] << 16) | (data[i * 4 + 2] << 8) | data[i * 4 + 3];
    }
    for (int i = 16; i < 64; i++)
    {
        W[i] = SIG1(W[i - 2]) + W[i - 7] + SIG0(W[i - 15]) + W[i - 16];
    }
    // 进行压缩
    for (int i = 0; i < 64; i++)
    {
        T1 = h + EP1(e) + Ch(e, f, g) + K[i] + W[i];
        T2 = EP0(a) + Ma(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }
    // 更新哈希值
    H[0] += a;
    H[1] += b;
    H[2] += c;
    H[3] += d;
    H[4] += e;
    H[5] += f;
    H[6] += g;
    H[7] += h;
}

// 将小端字节序转换为大端字节序
uint32_t toBigEndian(uint32_t value)
{
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

// 这个函数专门用于处理最后一个数据块，无论是seed，还是DB都不会满足56字节的要求
void sha256_final(uint8_t *seed, uint8_t bytesRead, uint8_t len, uint32_t tmp[8])
{
    uint8_t data[64] = {0};
    memcpy(data, seed, bytesRead);
    data[bytesRead] = 0x80;
    for (int i = bytesRead + 1; i < 56; i++)
    {
        data[i] = 0x00;
    }
    uint64_t length = len * 8;
    for (int i = 0; i < 8; i++)
    {
        data[56 + i] = (length >> (56 - i * 8)) & 0xff;
    }
    // 处理最后一个盘块
    sha256(data, tmp);
    // 把每一个算出来的值都转换为大端字节序
    for (int i = 0; i < 8; i++)
    {
        tmp[i] = toBigEndian(tmp[i]);
    }
}

void sha256_final_DB(uint8_t *data, uint8_t bytesRead, uint8_t len)
{
    data[bytesRead] = 0x80;
    for (int i = bytesRead + 1; i < 56; i++)
    {
        data[i] = 0x00;
    }
    uint64_t length = len * 8;
    for (int i = 0; i < 8; i++)
    {
        data[56 + i] = (length >> (56 - i * 8)) & 0xff;
    }
    // 处理最后一个盘块
    sha256_DB(data);
}

// OAEP填充时需要填充的Hash值
const uint8_t hash_0[32] =
    {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
        0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
        0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};

// 定义MGF1函数，存放掩码、消息、长度
void MGF1_seed(uint8_t *mask, uint8_t *seed, uint8_t len)
{
    // 由于我们的seed是36字节，所以直接进入sha256的填充环节
    // 需要将seed转换为uint8_t数组
    uint8_t seed_8[36]; // 因为后面要计数，所以多分配4个字节
    memcpy(seed_8, seed, 32);
    uint8_t times = (DB_len / 32) + 1; // 计算需要填充的次数
    uint32_t tmp[8] = {0};
    for (int i = 0; i < times; i++)
    {
        // 大端字节序
        seed_8[32] = (i >> 24) & 0xff;
        seed_8[33] = (i >> 16) & 0xff;
        seed_8[34] = (i >> 8) & 0xff;
        seed_8[35] = i & 0xff;
        sha256_final(seed_8, 36, len + 4, tmp);
        if (i == times - 1)
        {
            memcpy(mask + i * 32, tmp, 31);
            break;
        }
        memcpy(mask + i * 32, tmp, 32);
        // 打印当前的tmp
        // for (int i = 0; i < 8; i++)
        //{
        //    cout << hex << uppercase << setw(2) << setfill('0') << (int)tmp[i];
        //    if (i % 16 == 15)
        //        cout << endl;
        //    else
        //        cout << " ";
        //}
    }
}

void MGF1_DB(uint8_t *mask, uint8_t *meg, uint8_t len)
{
    // 一开始，仍是对512位数据进行处理
    uint8_t data[64] = {0};
    uint8_t times = (len / 64); // 需要进行3次循环
    for (size_t i = 0; i < times; i++)
    {
        memcpy(data, meg + i * 64, 64);
        sha256_DB(data);
    }
    // 最后一次循环
    memcpy(data, meg + times * 64, len % 64);
    // 进行计数填充
    data[31] = 0x00;
    data[32] = 0x00;
    data[33] = 0x00;
    data[34] = 0x00;

    sha256_final_DB(data, 35, len + 4);
    // 将hash值全部使用大数来存储
    for (int i = 0; i < 8; i++)
    {
        H[i] = toBigEndian(H[i]);
    }
    memcpy(mask, H, 32);
}

void OAEP(uint8_t pad, uint8_t len, uint64_t *data, uint8_t *EM)
{
    uint8_t DB[DB_len]; // 数据块
    // 首先，填充Hash值，这里默认为SHA-256中无消息输入的初始值
    memcpy(DB, hash_0, 32);
    // 然后，填充0x00
    for (int i = 0; i < pad; i++)
    {
        DB[32 + i] = 0x00;
    }
    memset(DB + 32, 0, pad);
    memset(DB + 32 + pad, 0x01, 1);
    // 最后，将数据填充到数据块中
    memcpy(DB + 32 + pad + 1, data, len);
    // 生成随机数种子
    // 将seed转换为uint8_t数组
    // uint8_t seed_8[32];
    // memcpy(seed_8, seed, 32);
    // 打印DB
    // for (int i = 0; i < DB_len; i++)
    //{
    //    cout << hex << uppercase << setw(2) << setfill('0') << (int)DB[i];
    //    if (i % 16 == 15)
    //        cout << endl;
    //    else
    //        cout << " ";
    //}
    uint8_t seed[32] = {
        0x84, 0xEE, 0x1D, 0x92, 0xBE, 0xFC, 0x55, 0x96,
        0x6F, 0x20, 0xB6, 0xFD, 0x18, 0xFC, 0x45, 0x20,
        0xCF, 0x17, 0x0B, 0xD8, 0x92, 0xE2, 0xE0, 0xD3,
        0x4F, 0xE7, 0xA0, 0x10, 0x17, 0xC8, 0x6B, 0x67};
    // uint8_t seed[32];
    // get_seed(seed);
    uint8_t DBmask[DB_len] = {0}; // 用于存储DB掩码
    MGF1_seed(DBmask, seed, 32);
    // 打印DBmask
    // for (int i = 0; i < DB_len; i++)
    //{
    //	cout << hex << uppercase << setw(2) << setfill('0') << (int)DBmask[i];
    //	if (i % 16 == 15)
    //		cout << endl;
    //	else
    //		cout << " ";
    //}

    // 得到DB掩码之后，进行异或操作
    for (int i = 0; i < DB_len; i++)
    {
        DB[i] ^= DBmask[i];
    }
    // 输出此时的DB
    // for (int i = 0; i < DB_len; i++)
    //{
    //	cout << hex << uppercase << setw(2) << setfill('0') << (int)DB[i];
    //	if (i % 16 == 15)
    //		cout << endl;
    //	else
    //		cout << " ";
    //}

    // 生成seed掩码
    uint8_t seedmask[32] = {0}; // 用于存储seed掩码
    MGF1_DB(seedmask, DB, 223);

    // 打印seedmask
    // for (int i = 0; i < 32; i++)
    //{
    //	cout << hex << uppercase << setw(2) << setfill('0') << (int)seedmask[i];
    //	if (i % 16 == 15)
    //		cout << endl;
    //	else
    //		cout << " ";
    //}
    // 得到seed掩码之后，进行异或操作
    for (int i = 0; i < 32; i++)
    {
        seed[i] ^= seedmask[i];
    }

    // 填充EM
    EM[0] = 0x00;
    memcpy(EM + 1, seed, 32);
    memcpy(EM + 33, DB, DB_len);
}

// 大数运算操作---------------------------------------------------------------------------------------------

uint32_t pow2[32] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,
                     16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152,
                     4194304, 8388608, 16777216, 33554432, 67108864, 134217728,
                     268435456, 536870912, 1073741824, 2147483648};

bool bigger(uint32_t a[128], uint32_t b[128])
{
    for (int i = 127; i >= 0; i--)
    {
        if (a[i] > b[i])
            return true;
        else if (a[i] < b[i])
            return false;
    }
    return false;
}

const uint64_t BASE = 0x100000000;

void sub(uint32_t res[128], uint32_t a[128], uint32_t b[128])
{
    uint64_t borrow = 0;
    uint32_t temp[128] = {0};
    for (int i = 0; i < 128; i++)
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

    for (int i = 0; i < 128; i++)
        res[i] = temp[i];
}

void mul(uint32_t res[128], uint32_t a[128], uint32_t b[128])
{
    uint32_t temp[256] = {0};

    for (int i = 0; i < 128; i++)
    {
        uint64_t carry = 0;
        for (int j = 0; j < 128; j++)
        {
            uint64_t sum = (uint64_t)a[i] * b[j] + temp[i + j] + carry;
            temp[i + j] = sum & 0xffffffff;
            carry = sum >> 32;
        }
        if (carry)
            temp[i + 128] += carry;
    }

    for (int i = 0; i < 128; i++)
        res[i] = temp[i];
}

int getBits(const uint32_t a[128])
{
    for (int i = 127; i >= 0; i--)
    {
        if (a[i])
            for (int j = 31; j >= 0; j--)
                if (a[i] & pow2[j])
                    return i * 32 + j;
    }

    return 0;
}

void div(uint32_t res[128], uint32_t a[128], uint32_t b[128])
{
    uint32_t temp[128] = {0};
    int n = getBits(b) / 32;
    int m = getBits(a) / 32 - n; // iterate times

    uint32_t d[128] = {0};
    d[0] = BASE / (b[n] + (uint64_t)1);
    uint32_t u_[129] = {0}, v_[128] = {0};
    uint64_t carry = 0;
    // scale a
    for (int i = 0; i < 128; i++)
    {
        uint64_t temp = (uint64_t)a[i] * d[0] + carry;
        u_[i] = temp & 0xffffffff;
        carry = temp >> 32;
    }
    if (carry)
        u_[128] = carry;
    // scale b
    mul(v_, b, d);

    int j = m;
    while (j >= 0)
    {
        uint32_t tem[129] = {0};
        for (int i = 0; i <= n + 1; i++)
            tem[i] = u_[i + j];
        uint64_t tem2 = (tem[n + 1] * BASE + tem[n]) / v_[n];
        tem2 = min(tem2, BASE - 1);

        uint32_t q_hat = static_cast<uint32_t>(tem2 & 0xffffffff);
        uint32_t qv[128] = {0};
        for (int i = 0; i < 128; i++)
        {
            uint64_t temp = (uint64_t)v_[i] * q_hat + carry;
            qv[i] = temp & 0xffffffff;
            carry = temp >> 32;
        }

        while (bigger(qv, tem))
        {
            q_hat--;
            sub(qv, qv, v_);
        }

        sub(tem, tem, qv);
        for (int i = 0; i <= n + 1; i++)
            u_[i + j] = tem[i];

        temp[j] = q_hat;

        j--;
    }

    for (int i = 0; i < 128; i++)
        res[i] = temp[i];
}

void mod(uint32_t res[128], uint32_t a[128])
{
    uint32_t temp[128] = {0};
    div(temp, a, P);
    mul(temp, temp, P);
    sub(res, a, temp);
}

void mod_mul(uint32_t res[128], uint32_t a[128], uint32_t b[128])
{
    mul(res, a, b);
    mod(res, res);
}

uint32_t ONE[128] = {1};
void mod_pow(uint32_t res[128], uint32_t a[128], uint32_t b[128])
{
    uint32_t temp[128] = {1};
    uint32_t base[128] = {0};
    for (int i = 0; i < 128; i++)
        base[i] = a[i];

    for (int i = 0; i < 128; i++)
    {
        // if (bigger(temp, ONE) && b[i] == 0)
        //     break;
        for (int j = 0; j < 32; j++)
        {
            if (b[i] & pow2[j])
                mod_mul(temp, temp, base);

            mod_mul(base, base, base);
        }
    }
    for (int i = 0; i < 128; i++)
        res[i] = temp[i];
}

void eto32(uint8_t seed[256], uint32_t m[128])
{
    for (int i = 0; i < 256; ++i)
    {
        m[i / 4] |= (seed[255 - i] << (8 * (i % 4)));
    }
}

int main()
{
    uint8_t tmp_256[256] = {0}; // 模数
    uint32_t e[128] = {0};      // 公钥
    uint8_t len;                // 消息长度
    // 进行数据读取
    {
        in.ignore(16); // 跳过16个字节
        in.read((char *)tmp_256, 256);
        for (int i = 0; i < 64; i++)
        {
            P[63 - i] = tmp_256[i * 4] | (tmp_256[i * 4 + 1] << 8) | (tmp_256[i * 4 + 2] << 16) | (tmp_256[i * 4 + 3] << 24);
        }
        in.read((char *)tmp_256, 256);
        for (int i = 0; i < 64; i++)
        {
            e[63 - i] = tmp_256[i * 4] | (tmp_256[i * 4 + 1] << 8) | (tmp_256[i * 4 + 2] << 16) | (tmp_256[i * 4 + 3] << 24);
        }
        in.ignore(256); // 跳过256个字
        in.read((char *)&len, 1);
    }
    // 这里为了方便，把输入的n和e的数组大小都设置为了128，但是我们需要的其实仅需64个，也就是读入的内容，需要把这个转换成小端序
    for (int i = 0; i < 64; i++)
    {
        P[i] = toBigEndian(P[i]);
        e[i] = toBigEndian(e[i]);
    }
    // cout << "P" << endl;
    //  for (int i = 0; i < 64; i++)
    //  {
    //      // 当前 uint32_t 数字分解成 4 个字节并逐个输出
    //      for (int byte = 0; byte < 4; byte++)
    //      { // 从高字节到低字节输出
    //          uint8_t current_byte = (P[i] >> (byte * 8)) & 0xFF;
    //          std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (int)current_byte << " ";
    //      }

    //     // 每 8 个字节换行
    //     if ((i + 1) % 8 == 0)
    //     {
    //         std::cout << std::endl;
    //     }
    // }

    uint64_t *data = new uint64_t[len / 8 + 1];
    in.read((char *)data, len);
    uint8_t EM[256]; // OAEP扩展后的数据
    uint8_t pad = DB_len - len - 1 - 32;
    OAEP(pad, len, data, EM);
    // 分段打印出EM的结果
    // for (int i = 0; i < 256; i++)
    // {
    //     cout << hex << uppercase << setw(2) << setfill('0') << (int)EM[i];
    //     if (i % 16 == 15)
    //         cout << endl;
    //     else
    //         cout << " ";
    // }
    // cout << "-----------------------------" << endl;
    // 将EM数组转换为uint32_t数组
    uint32_t EM_32[128] = {0};
    for (int i = 0; i < 64; i++)
    {
        EM_32[63 - i] = EM[i * 4] | (EM[i * 4 + 1] << 8) | (EM[i * 4 + 2] << 16) | (EM[i * 4 + 3] << 24);
    }
    for (int i = 0; i < 64; i++)
    {
        EM_32[i] = toBigEndian(EM_32[i]);
    }
    // 打印EM_32

    // for (int i = 0; i < 64; i++)
    // {
    //     // 当前 uint32_t 数字分解成 4 个字节并逐个输出
    //     for (int byte = 0; byte < 4; byte++)
    //     { // 从高字节到低字节输出
    //         uint8_t current_byte = (P[i] >> (byte * 8)) & 0xFF;
    //         std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (int)current_byte << " ";
    //     }

    //     // 每 8 个字节换行
    //     if ((i + 1) % 8 == 0)
    //     {
    //         std::cout << std::endl;
    //     }
    // }

    // 进行RSA加密
    uint32_t C[128] = {0};
    mod_pow(C, EM_32, e); // 参数分别为结果，加密的信息，公钥
    // 需要反转字节
    uint32_t C_final[64] = {0};
    for (int i = 0; i < 64; i++)
    {
        C_final[i] = (C[63 - i] >> 24) | ((C[63 - i] & 0x00ff0000) >> 8) | ((C[63 - i] & 0x0000ff00) << 8) | (C[63 - i] << 24);
    }
    out.write((char *)C_final, 256);

    //  打印C
    // for (int i = 0; i < 64; i++)
    // {
    //     // 当前 uint32_t 数字分解成 4 个字节并逐个输出
    //     for (int byte = 0; byte < 4; byte++)
    //     { // 从高字节到低字节输出
    //         uint8_t current_byte = (C_final[i] >> (byte * 8)) & 0xFF;
    //         std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << (int)current_byte << " ";
    //     }

    //     // 每 8 个字节换行
    //     if ((i + 1) % 8 == 0)
    //     {
    //         std::cout << std::endl;
    //     }
    // }

    return 0;
}