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
std::ifstream input("D:/Cryptolab/sha-256/dump.bin", std::ios::binary);
std::ofstream output("output.txt");
#define in input
#define out output
#endif

uint8_t buff[64] = {0}; // 用于存储读入的数据
uint64_t len = 0;       // 用于记录读入的数据长度

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
void sha256(uint8_t *data)
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

// 这个函数专门用于处理最后一个数据块
void sha256_final(uint8_t *data, uint8_t bytesRead, uint64_t len)
{
    // 进行填充
    if (bytesRead < 56)
    {
        data[bytesRead] = 0x80;
        std::fill(data + bytesRead + 1, data + 56, 0x00);
    }
    else // 此时同一个盘块无法再存储数据的长度，所以需要进行特殊的处理
    {
        data[bytesRead] = 0x80;
        std::fill(data + bytesRead + 1, data + 64, 0x00);
        // 以上的填充构成一个盘块
        // 先处理一遍哈希值
        sha256(data);
        std::fill(data, data + 56, 0x00); // 清空数据，仅剩下最后的8个字节来保存长度
    }
    // 保存数据长度
    uint64_t length = len * 8;
    for (int i = 0; i < 8; i++)
    {
        data[56 + i] = (length >> (56 - i * 8)) & 0xff;
    }
    // 处理最后一个盘块
    sha256(data);
    // for (int i = 0; i < 8; i++)
    // {
    //     for (int j = 3; j >= 0; j--)
    //     {
    //         // 提取每个字节并格式化为两位十六进制数
    //         cout << hex << uppercase << setw(2) << setfill('0') << ((H[i] >> (j * 8)) & 0xFF);
    //         if (i != 7 || j != 0)
    //             cout << " "; // 添加空格，最后一组数据后不添加
    //     }
    //     if (i % 2 == 1)
    //         cout << endl; // 每两组换行
    // }
    // 输出结果hash值
    for (int i = 0; i < 8; ++i)
    {
        uint32_t bigEndianValue = toBigEndian(H[i]);
        out.write(reinterpret_cast<const char *>(&bigEndianValue), sizeof(bigEndianValue));
    }
}

int main()
{
    // 读入数据，由于数据过大会占用内存，所以切合实际情况，每次读入512位数据，最后再进行填充
    uint8_t buffer[1024 * 48] = {0}; // 大缓冲区，一次读取 1024 字节
    uint8_t chunk[64] = {0};         // 小缓冲区，用于存储每次处理的 512 位（64 字节）数据
    size_t bytesRead = 0;
    size_t bufferIndex = 0;
    uint64_t total_len = 0; // 用于存储总读取的字节长度
    bool flag = true;

    while (flag)
    {
        // 如果缓冲区已经全部处理完，则重新读取数据到缓冲区
        if (bufferIndex == bytesRead)
        {
            in.read(reinterpret_cast<char *>(buffer), sizeof(buffer));
            bytesRead = in.gcount();
            bufferIndex = 0;
            if (bytesRead == 0)
            {
                flag = false;
                sha256_final(chunk, 0, total_len); // 调用 SHA-256 尾部处理函数
            }
            // 如果读到的数据不足 1024 字节，说明文件已经读到尾部
            if (bytesRead < sizeof(buffer))
            {
                flag = false;
            }
        }

        // 从缓冲区提取 64 字节数据块到 chunk
        while (bufferIndex + 64 <= bytesRead)
        {
            sha256(buffer + bufferIndex); // 对每块 64 字节的数据调用 SHA-256 处理函数
            bufferIndex += 64;
            total_len += 64;
        }

        // 处理文件尾部不足 64 字节的数据
        if (!flag && bufferIndex < bytesRead)
        {
            size_t remaining = bytesRead - bufferIndex;
            std::memcpy(chunk, buffer + bufferIndex, remaining);
            total_len += remaining;
            sha256_final(chunk, remaining, total_len); // 调用 SHA-256 尾部处理函数
        }
    }
}