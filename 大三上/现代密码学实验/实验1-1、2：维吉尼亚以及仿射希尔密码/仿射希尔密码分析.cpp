#include <iostream>
#include <vector>
#include <string>
#include <random>
using namespace std;

// 自定义函数生成随机数
int getRandomNumber(int min, int max)
{
    return min + rand() % (max - min + 1); // 生成 min 到 max 之间的随机数
}

// 获取去掉指定行和列后的子矩阵
vector<vector<int>> get_sub_matrix(const vector<vector<int>> &m, int n, int excludeRow, int excludeCol)
{
    vector<vector<int>> minorMatrix(n - 1, vector<int>(n - 1));
    int r = 0, c;

    for (int i = 0; i < n; i++)
    {
        if (i == excludeRow)
        {
            continue; // 跳过指定的行
        }
        c = 0;
        for (int j = 0; j < n; j++)
        {
            if (j == excludeCol)
            {
                continue; // 跳过指定的列
            }
            minorMatrix[r][c] = m[i][j];
            c++;
        }
        r++;
    }

    return minorMatrix;
}

// 递归计算行列式
int deta(const vector<vector<int>> &m, int n)
{
    if (n == 1)
    {
        return (m[0][0] % 26 + 26) % 26; // 1x1矩阵的行列式
    }

    if (n == 2)
    {
        // 2x2矩阵的行列式公式
        return ((m[0][0] * m[1][1] - m[0][1] * m[1][0]) % 26 + 26) % 26;
    }

    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        // 获取子矩阵
        vector<vector<int>> minorMatrix = get_sub_matrix(m, n, 0, i); // 去掉第 0 行和第 i 列

        // 递归计算子矩阵的行列式
        int cofactor = deta(minorMatrix, n - 1);

        // 符号为 (-1)^i，因此 alternation 依次为 1 和 -1
        int alternation = (i % 2 == 0) ? 1 : -1;

        // 计算当前项，并确保结果在 [0, 25] 范围内
        int currentTerm = (alternation * m[0][i] * cofactor) % 26;

        // 如果 currentTerm 为负数，加 26 再取模，保证非负
        if (currentTerm < 0)
        {
            currentTerm = (currentTerm + 26) % 26;
        }

        // 累加当前项
        sum = (sum + currentTerm) % 26;
    }

    // 确保 sum 非负
    return 26 - ((sum + 26) % 26);
}

// 计算模逆的函数，使用扩展欧几里得算法
int modInverse(int a, int mod)
{
    a = a % mod;

    for (int x = 1; x < mod; x++)
    {
        if ((a * x) % mod == 1)
        {

            return x;
        }
    }
    return -1; // 如果没有逆元，返回 -1
}

// 计算伴随矩阵
vector<vector<int>> adjugate_matrix(const vector<vector<int>> &m, int n)
{
    vector<vector<int>> adj(n, vector<int>(n));

    // 计算每个代数余子式
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            // 获取去掉第 i 行和第 j 列的子矩阵
            vector<vector<int>> minorMatrix = get_sub_matrix(m, n, i, j);

            // 计算子矩阵的行列式
            int cofactor = deta(minorMatrix, n - 1);

            // 符号为 (-1)^(i+j)
            if ((i + j) % 2 != 0)
            {
                cofactor = (-cofactor + 26) % 26;
            }

            // 将 cofactor 放入伴随矩阵的转置位置
            adj[j][i] = (cofactor + 26) % 26; // 注意是转置
        }
    }
    // 打印伴随矩阵，伴随矩阵测试成功
    // for(int i=0;i<3;i++)
    //  {
    //      for(int j=0;j<3;j++)
    //      {
    //          cout<<adj[i][j]<<" ";
    //      }
    //      cout<<endl;
    //  }

    return adj;
}

// 计算矩阵的逆
vector<vector<int>> inverse_matrix(const vector<vector<int>> &m, int n)
{
    // 1. 计算矩阵的行列式
    int det = deta(m, n);
    //cout << det << endl;
    if (det == 0)
    {
        return vector<vector<int>>();
        // throw invalid_argument("Matrix is not invertible, determinant is 0.");
    }

    // 2. 计算行列式在模26下的逆
    int det_inv = 0;
    det_inv = modInverse(det, 26);
    // cout << "det_inv" << det_inv << endl;
    if (det_inv == -1)
    {
        return vector<vector<int>>();
        // throw invalid_argument("Matrix is not invertible under mod 26.");
    }

    // 3. 计算伴随矩阵
    vector<vector<int>> adj = adjugate_matrix(m, n);
    // for(int i=0;i<3;i++)
    // {
    //     for(int j=0;j<3;j++)
    //     {
    //         cout<<adj[i][j]<<" ";
    //     }
    //     cout<<endl;
    // }

    // 4. 用行列式的逆乘以伴随矩阵的每个元素，并取模26
    vector<vector<int>> inv(n, vector<int>(n));
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            inv[i][j] = (adj[i][j] * det_inv) % 26;
            if (inv[i][j] < 0)
            {
                inv[i][j] += 26; // 确保非负
            }
        }
    }
    // //打印逆矩阵，逆矩阵测试成功
    // for(int i=0;i<3;i++)
    // {
    //     for(int j=0;j<3;j++)
    //     {
    //         cout<<inv[i][j]<<" ";
    //     }
    //     cout<<endl;
    // }

    return inv;
}

vector<vector<int>> group_by(string input1, int n)
{
    vector<vector<int>> group;
    int count = 0;
    vector<int> tmp;
    for (int i = 0; i < input1.length(); i++)
    {
        tmp.push_back((input1[i] - 'A') % 26);
        count++;
        // 这里默认如果不满足分组长度，则不进行添加
        if (count == n)
        {
            group.push_back(tmp);
            tmp.clear();
            count = 0;
        }
    }
    return group;
}

int process(vector<vector<int>> p, vector<vector<int>> c, int n)
{
    int len = p.size();
    for (int i = len - 1; i > 0; --i)
    {
        int j = getRandomNumber(0, i);
        swap(p[i], p[j]);
        swap(c[i], c[j]);
    }
    vector<vector<int>> pm, cm, pm_, cm_;
    for (int i = 0; i < n; i++)
    {
        pm.push_back(p[i]);
        pm_.push_back(p[i + n]);
        cm.push_back(c[i]);
        cm_.push_back(c[i + n]);
    }
    // 让对应的数组进行相减
    vector<vector<int>> pm_result, cm_result;
    for (int i = 0; i < n; i++)
    {
        vector<int> temp1, temp2;
        for (int j = 0; j < n; j++)
        {
            temp1.push_back((pm[i][j] - pm_[i][j] + 26) % 26);
            temp2.push_back((cm[i][j] - cm_[i][j] + 26) % 26);
        }
        pm_result.push_back(temp1);
        cm_result.push_back(temp2);
    }
    // 计算逆矩阵
    vector<vector<int>> pm_inv = inverse_matrix(pm_result, n);
    if (pm_inv.empty())
    {
        return -1;
    }

    // 计算密钥矩阵
    vector<vector<int>> key_matrix(n, vector<int>(n));

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            for (int k = 0; k < n; k++)
            {
                key_matrix[i][j] += pm_inv[i][k] * cm_result[k][j];
            }
            key_matrix[i][j] = (-(key_matrix[i][j] % 26) + 26) % 26;
        }
    }
    vector<int> offset;
    int t = 0;

    // 计算 key_matrix * pm[0]
    for (int i = 0; i < n; i++)
    {
        int sum = 0;
        for (int j = 0; j < n; j++)
        {
            // 矩阵乘法 key_matrix 的第 i 行和 pm 的第 0 行的点积
            sum += pm[0][j] * key_matrix[j][i];
        }

        // 计算偏移量，先减去矩阵乘积的结果，再取模
        int diff = cm[0][t] - sum;

        // 保证结果为正数
        offset.push_back((diff % 26 + 26) % 26);

        t++;
    }

    // 输出密钥矩阵
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            cout << key_matrix[i][j] << " ";
        }
        cout << endl;
    }
    // 输出偏移量
    for (int i = 0; i < n; i++)
    {
        cout << offset[i] << " ";
    }
    return 0;
}

int main()
{
    //int n = 4;
    // string plain,cipher;
    //string input1 = "THEQUICKLYBROWNFOXJUMPSOVERALAZYDOGE";
    //string input2 = "IOWWEMXSWRIKPPHJHTJQFQLLBZSQLXGUVYAB";
    int n;
    string input1, input2;
    cin >> n >> input1 >> input2;
    vector<vector<int>> p = group_by(input1, n);
    vector<vector<int>> c = group_by(input2, n);
    bool flag = true;
    while (flag)
    {
        int anwser = process(p, c, n);
        if (anwser == -1)
        {
            continue;
        }
        else
        {
            flag = false;
        }
    }

    return 0;
}
