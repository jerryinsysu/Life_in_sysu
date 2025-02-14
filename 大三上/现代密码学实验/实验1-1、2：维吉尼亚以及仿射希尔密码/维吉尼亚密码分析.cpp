#include <iostream>
#include <string>
#include <vector>
#include <cmath> // 用于 abs 函数
using namespace std;

constexpr double BIC = 0.061;
constexpr double std_frq[26] = {0.082, 0.015, 0.028, 0.043, 0.127, 0.022, 0.020, 0.061, 0.070, 0.002, 0.008, 0.040, 0.024, 0.067, 0.075, 0.019, 0.001, 0.060, 0.063, 0.091, 0.028, 0.010, 0.023, 0.001, 0.020, 0.001};

int GetKeylength(const string &ct);
vector<string> group_by(const string &ct, int step);
string GetKey(const string &ct, int keylength, const double wf[26]);

int main()
{
    string cyphertext, input;
    while (cin >> input)
    {
        cyphertext += input + ' ';
    }

    int keylength = GetKeylength(cyphertext);
    string key = GetKey(cyphertext, keylength, std_frq);
    int count = 0;

    // 解密过程
    for (int i = 0; i < cyphertext.size(); i++)
    {
        if (isalpha(cyphertext[i]))
        {
            // 计算密钥字母的偏移量（将 key[count % keylength] 转为 0-25 的数字）
            int key_offset = toupper(key[count % keylength]) - 'A';

            if (isupper(cyphertext[i]))
            {
                // 大写字母的解密公式
                cyphertext[i] = (cyphertext[i] - 'A' - key_offset + 26) % 26 + 'A';
            }
            else
            {
                // 小写字母的解密公式
                cyphertext[i] = (cyphertext[i] - 'a' - key_offset + 26) % 26 + 'a';
            }
            count++; // 只对字母进行计数
        }
    }

    cout << key << endl;
    cout << cyphertext << endl;
}

// 使用 vector<string> 替代动态分配的数组，避免手动内存管理
vector<string> group_by(const string &ct, int step)
{
    vector<string> group(step);
    int count = 0;

    for (char ch : ct)
    {
        if (isalpha(ch))
        {
            group[count] += ch;
            count = (count + 1) % step;
        }
    }

    return group;
}

// 尝试获取密钥的长度
int GetKeylength(const string &ct)
{
    double min_gap = 1;
    int keylength = 0;

    // 尝试不同的密钥长度，最多尝试 35 个字符的密钥
    for (int i = 1; i <= 35; i++)
    {
        vector<string> group = group_by(ct, i);
        double IC_total = 0;

        for (const auto &str : group)
        {
            // 统计每个字母的频率
            int frq[26] = {0};
            int packet_size = str.size();
            for (char ch : str)
            {
                frq[toupper(ch) - 'A']++;
            }

            // 计算每个分组的 IC
            int sum = 0;
            for (int l = 0; l < 26; l++)
            {
                sum += frq[l] * (frq[l] - 1);
            }

            if (packet_size > 1)
            {
                IC_total += static_cast<double>(sum) / (packet_size * (packet_size - 1));
            }
        }

        double avg_ic = IC_total / i;
        if (avg_ic >= BIC)
        {
            min_gap = abs(avg_ic - BIC);
            keylength = i;
            break;
        }
    }

    return keylength;
}

// 获取密钥
string GetKey(const string &ct, int keylength, const double wf[26])
{
    vector<string> group = group_by(ct, keylength); // 分组
    string keystring = "";

    // 对每一组进行分析，推测密钥
    for (const auto &str : group)
    {
        int frq[26] = {0};
        for (char ch : str)
        {
            frq[toupper(ch) - 'A']++;
        }

        // 使用拟重合指数(IC)推测密钥字母
        double max_ic = 0.0;
        char key = 'A';

        // 尝试每一个可能的密钥字母（0 到 25 位移量）
        for (int g = 0; g < 26; g++)
        {
            double ic = 0.0;
            int total_count = str.size();

            // 计算当前位移(g)下的拟重合指数
            for (int i = 0; i < 26; i++)
            {
                ic += wf[i] * frq[(i + g) % 26];
            }

            ic /= total_count;

            // 更新最大拟重合指数，推测最可能的密钥字母
            if (ic > max_ic)
            {
                max_ic = ic;
                key = 'A' + g;
            }
        }

        keystring += key;
    }

    return keystring;
}
