#include <chrono>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <vector>
#define green "\033[1;32m"
#define reset "\033[0m"

int hash_value_(const std::string &wprefix);

void buildArrayIndex(std::ifstream &index_file);
// std::ifstream file_L("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/korpus
//std::ifstream index_file ("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/rawindex.txt");
int main(int argc, char *argv[]) {
    std::ifstream index_file("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/rawindex.txt");
    buildArrayIndex(index_file);
    return 0;
}

void buildArrayIndex(std::ifstream &index_file) {
    std::cout << "Building index..." << std::endl;
    std::ofstream outFile("data.bin", std::ios::binary);
    std::string line, current_prefix, previous_prefix;
    size_t current_index = 0;
    size_t start_index = 0;
    while (std::getline(index_file, line)) {
        current_prefix = line.substr(0, line.find(' '));
        current_prefix = current_prefix.substr(0, 3);

        if (current_prefix != previous_prefix) {
            if (!previous_prefix.empty()) {
                size_t end_index = current_index;
                outFile.seekp(hash_value_(previous_prefix) * 2 * sizeof(size_t));
                outFile.write(reinterpret_cast<const char *>(&start_index), sizeof(start_index));
                outFile.write(reinterpret_cast<const char *>(&end_index), sizeof(end_index));
            }
            start_index = current_index;
            previous_prefix = current_prefix;
        }
        current_index += line.length() + 1;
    }


    if (!previous_prefix.empty()) {
        outFile.seekp(hash_value_(previous_prefix) * 2 * sizeof(size_t));
        outFile.write(reinterpret_cast<const char *>(&start_index), sizeof(start_index));
        outFile.write(reinterpret_cast<const char *>(&current_index),
                      sizeof(current_index));
    }

    outFile.close();
    index_file.clear();
    std::cout << "Index built" << std::endl;
}

int hash_value_(const std::string &wprefix) {
    int n = wprefix.length();
    int value = 0;
    if (n >= 3) {
        value = (static_cast<unsigned char>(wprefix[0]) * 900 + static_cast<unsigned char>(wprefix[1]) * 30 +
                 static_cast<unsigned char>(wprefix[2]));
    } else if (n == 2) {
        value = (static_cast<unsigned char>(wprefix[0]) * 30 + static_cast<unsigned char>(wprefix[1]));
    } else if (n == 1) {
        value = (static_cast<unsigned char>(wprefix[0]));
    }
    return value;
}
