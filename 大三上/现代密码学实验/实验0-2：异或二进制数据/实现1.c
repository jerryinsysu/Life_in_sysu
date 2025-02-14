#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

int main() {
#ifdef ONLINE_JUDGE
    // 在在线评测系统中，从 stdin 读取输入，输出到 stdout
    #ifdef _WIN32
    setmode(fileno(stdin), O_BINARY);
    setmode(fileno(stdout), O_BINARY);
    #endif

    char byte;
    char mask;
    fread(&mask, 1, 1, stdin);
    // printf("mask=%02X\n", mask);
    for(int i =0;i<4;i++)
    {
        
        fread(&byte, 1, 1, stdin);
    }
    while (fread(&byte, 1, 1, stdin) == 1) {
         byte = byte ^ mask;
         //printf("%02X ", byte); // 使用%02X格式化输出16进制数，每个字节占两位
         fwrite(&byte, 1, 1, stdout);
    }

    
    // 读取二进制数据
    // unsigned char buffer[1024];
    // size_t bytesRead;
    
    // while ((bytesRead = fread(buffer, 1, sizeof(buffer), stdin)) > 0) {
    //     fwrite(buffer, 1, bytesRead, stdout);
    // }

#else
    // 在本地调试时，从文件读取输入和输出
    printf("Debugging mode\n");
    FILE *inputFile = fopen("dump.bin", "rb");
    FILE *outputFile = fopen("out.bin", "wb");
    
    
    if (inputFile == NULL || outputFile == NULL) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }
    char byte; // 读取二进制数据
    char mask;
    fread(&mask, 1, 1, inputFile);
    printf("mask=%02X\n", mask);
    for(int i =0;i<4;i++)
    {
        fread(&byte, 1, 1, inputFile);
    }
    
    while (fread(&byte, 1, 1, inputFile) == 1) {
         byte = byte ^ mask;
         printf("%02X ", byte); // 使用%02X格式化输出16进制数
         fwrite(&byte, 1, 1, outputFile);
    }
    
    fclose(inputFile);
    fclose(outputFile);
#endif

    return 0;
}