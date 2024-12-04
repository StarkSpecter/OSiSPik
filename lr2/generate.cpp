//#include <windows.h>
//#include <algorithm>
//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <string>
//#include <random>
//#include <sstream>
//
//#define ARRAY_SIZE 10000
//#define NUM_ARRAYS 10
//#define FILE_NAME L"arrays.txt"
//
//void generateRandomArrays() {
//    std::wofstream outFile(FILE_NAME);
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<> dis(10000, 99999);
//
//    for (int i = 0; i < NUM_ARRAYS; ++i) {
//        for (int j = 0; j < ARRAY_SIZE; ++j) {
//            outFile << dis(gen);
//            if (j < ARRAY_SIZE - 1) {
//                outFile << L" ";
//            }
//        }
//        outFile << L"\n";
//    }
//    outFile.close();
//}
//
//int main() {
//    generateRandomArrays();
//    std::wcout << L"Random arrays generated and saved to " << FILE_NAME << L"." << std::endl;
//    return 0;
//}
