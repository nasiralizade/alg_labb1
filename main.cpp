#include <fstream>
#include <iostream>
#include <vector>
#include <istream>
// Define the alphabet in Latin-1 order
#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÖ"
#define green "\033[1;32m"
#define reset "\033[0m"

static unsigned char u2l[256];

void initialize() {
    unsigned char ch;

    for (unsigned char & i : u2l)
        i = 0;

    for (unsigned char *s = (unsigned char *) ALPHABET; *s; s++) {
        ch = *s + 'a' - 'A';
        u2l[*s] = u2l[ch] = ch;
    }

    // Special handling for accented characters
    ch = 223; // German double-s
    u2l[ch] = 's';

    for (ch = 224; ch <= 227; ++ch) // a with accent (except å and ä)
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'a';

    ch = 230; // ae to ä
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'ä';

    ch = 231; // c with cedilla to c
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'c';

    for (ch = 232; ch <= 235; ++ch) // e with accent (including é)
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'e';

    for (ch = 236; ch <= 239; ++ch) // i with accent
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'i';

    ch = 240; // eth to d
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'd';

    ch = 241; // n with ~ to n
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'n';

    for (ch = 242; ch <= 245; ++ch) // o with accent (except ö)
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'o';

    ch = 248; // o with stroke to ö
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'ö';

    for (ch = 249; ch <= 252; ++ch) // u with accent
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'u';

    ch = 253; // y with accent
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'y';
    ch = 255;
    u2l[ch] = 'y';
}

std::string to_lower(const std::string &str) {
    std::string result;
    for (unsigned char c: str) {
        result += u2l[c] ? u2l[c] : c;
    }
    return result;
}

int hash_value_(std::string &wprefix) {
    int n = wprefix.length();
    if (n >= 3) {
        return static_cast<unsigned char>(wprefix[0]) * 900 + static_cast<unsigned char>(wprefix[1]) * 30 + static_cast<
                   unsigned char>(wprefix[2]) % 27000;
    }
    if (n == 2) {
        return static_cast<unsigned char>(wprefix[0]) * 30 + static_cast<unsigned char>(wprefix[1]) % 27000;
    }
    if (n == 1) {
        return static_cast<unsigned char>(wprefix[0]) % 27000;
    }
    return -1;
}

std::vector<int> buildArrayIndex(std::ifstream &index_file, std::vector<int> &A) {
    std::string line, wprefix, temp;
    size_t index_i = 0;
    while (getline(index_file, line)) {
        wprefix = line.substr(0, line.find(' '));
        if (wprefix != temp) {
            A[hash_value_(wprefix)] = index_i;
            temp = wprefix;
        }
        index_i += line.size()+1;
    }
    index_file.clear();
    return A;
}

std::vector<int> my_search(const std::vector<int> &A,std::ifstream &index_file, const std::string &w) {
    std::vector<int> matches;
    std::string wprefix = w.substr(0, 3);
    int i = A[hash_value_(wprefix)];
    int j = A[hash_value_(wprefix) + 1];
    if (i == -1) { return matches; }
    int m ;
    std::string s;
    while (j - i > 1000) {
        m = (i + j) / 2;
        index_file.seekg(m);
        index_file >> s;
        if (!isalpha(s[0])) {
            while (!isalpha(s[0])) {
                m++;
                index_file.seekg(m);
                index_file >> s;
            }
        }
        if (s < w) {
            i = m;
        } else {
            j = m;
        }
    }
    index_file.seekg(i);
    int x;
    while (true) {
        index_file >> s;
        if (s == w) {
            index_file >> x;
            matches.push_back(x);
            if (matches.size() == 25 || index_file.eof()) {
                return matches;
            }
        }
        else if (s > w) {
            break;
        }
    }
    return matches;
}

void store_time_search( std::chrono::duration<long long, std::ratio<1, 1000>>::rep rep, const std::string & search);

int main(int argc, char *argv[]) {
    initialize();
    std::ifstream file_L("korpus.txt");
    std::ifstream index_file ("rawindex.txt");
    if(!file_L.is_open() || !index_file.is_open()) {
        std::cerr << "Error: cannot open file " << std::endl;
    }
    std::vector<int> A(30 * 30 * 30, -1);
    auto Array_index = buildArrayIndex( index_file, A);
    std::string search;
    if (argc > 1) {
        search = argv[1];
    } else {
        std::cout << "Mata in ordet du vill söka efter: ";
        std::cin >> search;
    }
    auto start = std::chrono::high_resolution_clock::now();
    auto index_L = my_search(Array_index,index_file, to_lower(search));
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() <<
            " milliseconds" << std::endl;
    if (index_L.empty()) {
        std::cout << "The word is not found" << std::endl;
    } else {
        std::cout << "Det finns " << index_L.size() << " förekomster av ordet. " << std::endl;
        std::string print_out;
        std::cout<<"Vill du skriva ut förekomsterna? (y/n): ";
        std::cin>>print_out;
        if (print_out != "y") {
            return 0;
        }
        std::cout<<"---------------------------------------------------\n";
        int antal = 15;
        for (int i: index_L) {
            char before[antal], after[antal];
            file_L.seekg(i - antal);
            file_L.read(before, antal);
            file_L.seekg(i + search.size());
            file_L.read(after, antal);
            std::cout << "..." << before <<green<< search<<reset << after << "..." << std::endl;
        }
        store_time_search(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(),search);
        std::cout << std::endl;
    }
    index_file.close();
    file_L.close();
    return 0;
}


void store_time_search(std::chrono::duration<long long, std::ratio<1, 1000>>::rep rep, const std::string & search) {
    std::ofstream file("time_search.txt", std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << std::endl;
    }
    file << search << "\t" << rep <<" ms\n";
    file.close();
}
