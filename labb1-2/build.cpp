#include <chrono>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <vector>
#define green "\033[1;32m"
#define reset "\033[0m"
std::ifstream file_L("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/korpus");

int hash_value_(const std::string &wprefix);

void buildArrayIndex(std::ifstream &index_file);

void My_toLower(std::string &w);

std::vector<int> my_search(std::ifstream &index_file, std::ifstream &inFile, const std::string &w);

void my_print(const std::vector<int> &v, const std::string &w);
// std::ifstream file_L("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/korpus
//std::ifstream index_file ("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/rawindex.txt");
int main(int argc, char *argv[]) {
    std::ifstream index_file("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/rawindex.txt");
    buildArrayIndex(index_file);
return 0;
    std::ifstream inFile("data.bin", std::ios::binary);
    if (!file_L.is_open() || !index_file.is_open() || !inFile.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    std::string w;
    if (argc > 1) {
        w = argv[1];
    } else {
        std::cout << "Mata in ordet du vill söka efter: ";
        std::cin >> w;
        My_toLower(w);
    }
    auto matches = my_search(index_file, inFile, w);
    std::cout << "Antal förekomster av ordet " << w << " är: " << matches.size() << std::endl;
    my_print(matches, w);
    file_L.clear();

    return 0;
}

std::vector<int> my_search(std::ifstream &index_file, std::ifstream &inFile, const std::string &w) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> matches;
    std::string wprefix = w.substr(0, 3);

    if (!index_file.is_open() || !inFile.is_open()) {
        std::cerr << "Error: Files not open for reading." << std::endl;
        return matches;
    }

    size_t i = 0, j = 0;
    long long wp = hash_value_(wprefix);

    inFile.seekg(wp * 2 * sizeof(size_t));
    if (!inFile.read(reinterpret_cast<char *>(&i), sizeof(i))) {
        std::cerr << "Error reading start index from data.bin at position " << wp * 2 * sizeof(size_t) << std::endl;
        return matches;
    }
    if (!inFile.read(reinterpret_cast<char *>(&j), sizeof(j))) {
        std::cerr << "Error reading end index from data.bin at position " << (wp * 2 * sizeof(size_t) + sizeof(size_t))
                  << std::endl;
        return matches;
    }
    index_file.seekg(i, std::ios::beg);
    std::string line;
    long long m;
    std::string s;
    while (j-i>1000) {
        m= (i+j)/2;
        index_file.seekg(m, std::ios::beg);
        std::getline(index_file, line);

        index_file>> s;
       if (s < w) {
            i=m;
        } else {
            j=m;
        }
    }
    index_file.seekg(i, std::ios::beg);
    while (true) {
        index_file>>s;
        if (index_file.peek() == EOF) {
            break;
        }
        if (s==w) {
            int x;
            index_file>>x;
            matches.push_back(x);
            if (matches.size()==25) {
                auto end = std::chrono::high_resolution_clock::now();
                std::cout<<"taken time: "<<std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()<<"\n";

            }
        } else if (s > w) {
            break;
        }
    }

    /*while (index_file.tellg() < j) {
        if (index_file.peek() == EOF) {
            break;
        }
        std::getline(index_file, line);
        std::istringstream iss(line);
        std::string current_word;
        iss >> current_word;
        if (current_word == w) {
            int x;
            iss >> x;
            matches.push_back(x);
            if (matches.size() == 25) {
                auto end = std::chrono::high_resolution_clock::now();
                auto taken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                std::cout << taken << " ms" << std::endl;
            }
        } else if (current_word > w) {
            break;
        }
    }*/

    return matches;
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

    // Handle the last prefix: write its start and end indices.
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

void My_toLower(std::string &w) {
    const std::locale loc("sv_SE.UTF-8"); // Swedish locale
    try {
        for (char &c: w) {
            c = std::tolower(c, loc);
        }
        std::cout << "Searching for: " << w << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void my_print(const std::vector<int> &v, const std::string &w) {
    int count = 0;
    int antal = 30;
    std::string ask;
    std::string before(antal, ' ');
    std::string after(antal, ' ');
    std::string word(w.size(), ' ');
    for (const auto i: v) {
        file_L.seekg(i - antal);
        file_L.read(&before[0], antal);
        file_L.read(&word[0], w.size());
        file_L.read(&after[0], antal);
        std::cout << before << green << word << reset << after << std::endl;
        count++;
        if (count == 25 && v.size() > 25) {
            std::cout << "Vill du skriva ut resten? (j/n): ";
            std::cin >> ask;
            if (ask == "n") {
                break;
            }
        }
    }
}
