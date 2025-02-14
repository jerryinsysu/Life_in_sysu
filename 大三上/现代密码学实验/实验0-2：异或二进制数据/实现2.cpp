#include <iostream>
#include <fstream>
#include <stdint.h>
#ifdef ONLINE_JUDGE
    #define in cin
    #define out cout
#else
    std::ifstream input("input.bin", std::ios::binary);
    std::ofstream output("output.txt");
    #define in input
    #define out output
#endif

using namespace std;

int main(void)
{
    uint8_t key;
    uint32_t len;
    in.read((char*)&key, sizeof(key));
    in.read((char*)&len, sizeof(len));

    uint8_t* data= new uint8_t[len];
    for(int i=0; i<len; i++)
        in.read((char*)&data[i], sizeof(data[i]));

    uint8_t* res=new uint8_t[len];
    for(int i=0; i<len; i++)
        res[i]=data[i]^key;
    
    for(int i=0; i<len; i++)
        out.write((char*)&res[i], sizeof(res[i]));

    delete[] data;
    delete[] res;

    return 0;
}