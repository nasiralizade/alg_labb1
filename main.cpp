#include <fstream>
#include <iostream>
#include <vector>
#include <istream>

int hash_value_(std::string wprefix) {
    int n=wprefix.length();
    if (n>=3) {
        return (wprefix[0]*900+wprefix[1]*30+wprefix[2])%27000;
    }if (n==2) {
        return (wprefix[0]*30+wprefix[1])%27000;
    }if (n==1) {
        return wprefix[0]%27000;
    }
    return -1;
}
std::vector<int> buildArrayIndex(const std::string & filename ,std::vector<int> & A) {
    std::ifstream file_index(filename);
    if (!file_index) {
        std::cerr << "Error: cannot open file " << filename << std::endl;
        exit(1);
    }
    std::string line,wprefix,temp;
    size_t index_i=0;
    while (getline(file_index,line)) {
        wprefix=line.substr(0,line.find(' '));
        if (wprefix!=temp) {
            A[hash_value_(wprefix)]=index_i;
            temp=wprefix;
        }
        index_i+=line.size();
        }
    return A;
}

std::vector<int> my_search(const std::vector<int> & A, const std::string & w) {
    std::vector<int>matches;
    std::ifstream file_index("text.txt");
    std::string wprefix=w.substr(0,3);
    int i=A[hash_value_(wprefix)];
    int j=A[hash_value_(wprefix)+1];
    int m=0;
    std::string s;
    while (j-i>1000) {
        m=floor((i+j)/2);
        file_index.seekg(m);
        file_index>>s;
        if (s<=w) {
            i=m;
        }else {
            j=m;
        }
    }
    if (i==-1){return matches;}
    file_index.seekg(i);
    int x;
    while (true) {
        file_index>>s;
        if (s==w) {
            file_index>>x;
            matches.push_back(x);
            if (matches.size()==25) {
                return matches;
            }
        }
        if (s>w) {
            return matches;
        }
    }
}

std::string my_to_lower(const std::string & str) {
    std::string str2;
    for (int i=0;i<str.size();i++) {
        str2+=std::tolower(str[i]);
    }
    return str2;
}

int main(int argc, char *argv[])
{
    std::ifstream file_L("org.txt");
    std::string filename="text.txt";


    std::vector<int>A(30*30*30,-1);
    auto Array_index=buildArrayIndex(filename,A);
    std::string search;
    if (argc>1) {
        search=argv[1];
    }else {
        std::cout<<"Enter the word you want to search: ";
        std::cin>>search;
    }
    auto search2=my_to_lower(search);
    auto index_L=my_search(Array_index,search2);
    if (index_L.empty()) {
        std::cout<<"The word is not found"<<std::endl;
    }else {
        std::cout<<"Det finns "<<index_L.size()<<" förekomster av ordet."<<std::endl;
        for (int i=0;i<index_L.size();i++) {
            file_L.seekg(index_L[i]-30);
            for (int i=0; i<90;i++) {
                char c;
                file_L.get(c);
                std::cout<<c;
            }
            std::cout<<std::endl;
        }

        std::cout<<std::endl;
    }

    return 0;
}
