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

    for (unsigned char &i: u2l)
        i = 0;

    for (unsigned char *s = (unsigned char *) ALPHABET; *s; s++) {
        ch = *s + 'a' - 'A';
        u2l[*s] = u2l[ch] = ch;
    }

    // Special handling for accented characters
    ch = 223; // German double-s
    u2l[ch] = 's';

    for (ch = 224; ch <= 227; ++ch) // a with accent (except ? and ?)
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'a';

    ch = 230; // ae to ?
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'e';

    ch = 231; // c with cedilla to c
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'c';

    for (ch = 232; ch <= 235; ++ch) // e with accent (including ?)
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'e';

    for (ch = 236; ch <= 239; ++ch) // i with accent
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'i';

    ch = 240; // eth to d
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'd';

    ch = 241; // n with ~ to n
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'n';

    for (ch = 242; ch <= 245; ++ch) // o with accent (except ?)
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'o';

    ch = 248; // o with stroke to ?
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
    std::cout<<"searching for: "<<result<<std::endl;
    return result;
}

int hash_value_(std::string &wprefix) {
    int n = wprefix.length();
    int value = 0;
    if (n >= 3) {
        value = (static_cast<unsigned char>(wprefix[0]) * 31 * 31 + static_cast<unsigned char>(wprefix[1]) * 31 +
                 static_cast<
                     unsigned char>(wprefix[2]));
    }
    if (n == 2) {
        value = (static_cast<unsigned char>(wprefix[0]) * 31 + static_cast<unsigned char>(wprefix[1]));
    }
    if (n == 1) {
        value = (static_cast<unsigned char>(wprefix[0]));
    }
    return value % 27000;
}

std::vector<int> buildArrayIndex(std::vector<int> &A, std::ifstream &index_file) {
    std::string line, wprefix, temp;
    size_t index_i = 0;
    while (getline(index_file, line)) {
        wprefix = line.substr(0, line.find(' '));
        wprefix = wprefix.substr(0, 3);
        if (wprefix != temp) {
            A[hash_value_(wprefix)] = index_i;
            temp = wprefix;
        }

        index_i += line.length() + 1;
    }


    index_file.clear();
    return A;
}

std::vector<int> my_search(const std::vector<int> &A, std::ifstream &index_file, const std::string &w) {
    std::vector<int> matches;
    std::string wprefix = w.substr(0, 3);
    long long i = A[hash_value_(wprefix)];
    long long j = A[hash_value_(wprefix) + 1];

    if (i == -1) { return matches; }

    index_file.seekg(0, std::ios::end);
    std::streampos file_size = index_file.tellg();
    if (j == -1 || j > file_size) {
        j = file_size;
    }
    long long m = 0;
    std::string s;
    while (j - i > 1000) {
        m = (i + j) / 2;
        index_file.seekg(m,std::ios::beg);
        getline(index_file, s);
        index_file >> s;
        if (s < w) {
            i = m;
        } else {
            j = m;
        }
    }
    index_file.seekg(i, std::ios::beg);
    int x;
    std::string linee;
    while (true) {
        if (matches.size() == 25 || index_file.eof()) {
            return matches;
        }
        getline(index_file,s);
        index_file >> s;
        if (s == w) {
            index_file >> x;
            matches.push_back(x);
        } else if (s > w) {
            break;
        }
    }
    return matches;
}


int main(int argc, char *argv[]) {
    initialize();
    std::vector<int> A(30 * 30 * 30, -1);
    std::ifstream file_L("korpus.txt");
    std::ifstream index_file("rawindex.txt");
    if (!file_L.is_open() || !index_file.is_open()) {
        std::cerr << "Error: cannot open file " << std::endl;
    }

    std::string search;
    auto Array_index = buildArrayIndex(A, index_file);
    if (argc > 1) {
        search = argv[1];
    } else {
        std::cout << "Mata in ordet du vill soka efter: ";
        std::cin >> search;
        search = to_lower(search);
    std::cout << hash_value_(search)<<" :"<<Array_index[hash_value_(search)] << std::endl;
    }
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto index_L = my_search(Array_index, index_file, to_lower(search));
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
            << "[ms]" << std::endl;

    if (index_L.empty()) {
        std::cout << "The word is not found" << std::endl;
    } else {
        std::cout << "Det finns " << index_L.size() << " forekomster av ordet. " << std::endl;
        std::string print_out;
        std::cout << "Vill du skriva ut forekomsterna? (y/n): ";
        std::cin >> print_out;
        if (print_out != "y") {
            return 0;
        }
        std::cout << "---------------------------------------------------\n";
        int antal = 15;
        for (int i: index_L) {
            char before[antal], after[antal];
            char word[search.size()];
            file_L.seekg(i - antal);
            file_L.read(before, antal);
            file_L.read(word, search.size());
            file_L.read(after, antal);
            std::cout << before << green << word << reset << after << std::endl;
        }

        std::cout << std::endl;
    }
    index_file.close();
    file_L.close();
    return 0;
}
