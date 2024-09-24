#include <chrono>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <vector>
// Define the alphabet in Latin-1 order
#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÖ"
#define green "\033[1;32m"
#define reset "\033[0m"
std::ifstream file_L("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/korpus");
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

    for (ch = 224; ch <= 227; ++ch) // a with accent (except ? and ?)
        u2l[ch + 'A' - 'a'] = u2l[ch] = 'a';

    ch = 230; // ae to ?
    u2l[ch + 'A' - 'a'] = u2l[ch] = 'ä';

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
        //result+= static_cast<unsigned char>(std::tolower(c));
        result += u2l[c] ? u2l[c] : c;
    }
    std::cout<<"Searching for: "<<result<<"..."<<std::endl;
    return result;
}
int hash_value_(const std::string &wprefix);

void buildArrayIndex(std::ifstream &index_file);



std::vector<int> my_search(std::ifstream &index_file, std::ifstream &inFile, const std::string &w);

void my_print(const std::vector<int> &v, const std::string &w);
// path till filerna i labbsalen
// std::ifstream file_L("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/korpus
//std::ifstream index_file ("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/rawindex.txt");
int main(int argc, char *argv[]) {
	initialize();
    std::ifstream index_file("/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/rawindex.txt");
    //buildArrayIndex(index_file); // kör denna bara denna del för att skapa data.bin file utan search, annars kör utan den
    //return 0;
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
    }
    w = to_lower(w);
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
    if (!inFile.read(reinterpret_cast<char *>(&i), sizeof(i))) { // läs in första position och spara i "i"
        std::cerr << "Error reading start index from data.bin at position " << wp * 2 * sizeof(size_t) << std::endl;
        return matches;
    }
    if (!inFile.read(reinterpret_cast<char *>(&j), sizeof(j))) { // nästa ord
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
