#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <iostream>
#include <array>
#include <vector>
#include <cassert>
#include <algorithm>

std::array<FILE*, 2> file;
std::string file_name = "in";
const size_t block_size = 1000;
std::array<std::vector<int64_t>, 2> tmp;

void read_to_vec(size_t pos, int thread_num) {
    size_t n = tmp[thread_num].size();
    fseek(file[thread_num], pos*sizeof(int64_t), 0);
    fread(&tmp[thread_num][0], sizeof(int64_t), n, file[thread_num]);
}

void rewrite_file(size_t pos, size_t n, int thread_num) {
    fseek(file[thread_num], pos*sizeof(int64_t), 0);
    fwrite(&tmp[thread_num][0], sizeof(int64_t), n, file[thread_num]);
}

void find(int64_t value, size_t beg, size_t end, size_t &l, int thread_num) {
    fseek(file[thread_num], beg*sizeof(int64_t), 0);
    for (int i = beg; i < end; i++) {
        int64_t val;
        fread(&val, sizeof(int64_t), 1, file[thread_num]);
        if (val <= value) l++;
    }
}

void swap_in_file(int64_t value1, int64_t value2, size_t place1, size_t place2, int thread_num) {
    fseek(file[thread_num], place1*sizeof(int64_t),0);
    fwrite(&value2,sizeof(int64_t),1,file[thread_num]);
    fseek(file[thread_num], place2*sizeof(int64_t),0);
    fwrite(&value1,sizeof(int64_t),1,file[thread_num]);
    for (size_t i = 0; i < file.size(); ++i) {
        fclose(file[i]);
        file[i] = fopen(file_name.c_str(), "rb+");
    }
}

void swap(size_t pos1, size_t pos2) {
    int64_t elem = tmp[0][pos1];
    tmp[0][pos1] = tmp[1][pos2];
    tmp[1][pos2] = elem;
}

// >
void find_bigger(int64_t value, size_t& it, int thread_num) {
    while (it < tmp[thread_num].size()) {
        if (tmp[thread_num][it] > value)
            break;
        else it++;
    }
}
// <=
void find_less(int64_t value, size_t& it, int thread_num) {
    while (it < tmp[thread_num].size()) {
        if (tmp[thread_num][it] <= value)
            break;
        else it++;
    }
}

std::vector<int64_t> get_vector(const std::string& file_name) {
    FILE* file = fopen(file_name.c_str(), "rb");
    fseek(file, 0, 2);
    size_t n = ftell(file) / sizeof(int64_t);
    fseek(file, 0, 0);
    std::vector<int64_t> v(n);
    for (int i = 0; i < n; ++i) {
        fread(&v[i], sizeof(v[i]), 1, file);
        //        std::cerr << v[i] << ' ';
    }
    //    std::cerr << std::endl;
    fclose(file);
    return v;
}

void sort (size_t beg, size_t end) {
    if (end > beg + 1) {
        //std::cerr << "sort: " << beg << ' ' << end << std::endl;
        size_t d = rand() % (end-beg);
        fseek(file[0], (beg+d)*sizeof(int64_t),0);
        int64_t value;
        fread(&value,sizeof(int64_t),1,file[0]);
        size_t l1=0,l2=0,r1=0,r2=0;
        std::thread t1 (find, value, beg, beg+(end-beg)/2, std::ref(l1), 0);
        std::thread t2 (find, value, beg+(end-beg)/2, end, std::ref(l2), 1);
        t1.join();
        t2.join();
        size_t l=l1+l2-1;
        //std::cerr << "sort l: " << l << " r: " << r << std::endl;
        int64_t replaced;
        fseek(file[0], (beg+l)*sizeof(int64_t), 0);
        fread(&replaced,sizeof(int64_t), 1, file[0]);
        swap_in_file(value,replaced,beg+d,beg+l, 0);
        //get_vector("in");
        size_t s1 = 0, s2 = 0;
        size_t r = (end-beg)-l-1;
        while ((s1 < l) && (s2 < r)) {
            size_t it1 = 0, it2 = 0;
            tmp[0].resize(std::min(block_size, l - s1));
            tmp[1].resize(std::min(block_size, r - s2));
            std::thread t1 (read_to_vec, beg+s1, 0);
            std::thread t2 (read_to_vec,beg+l+1+s2, 1);
            t1.join();
            t2.join();
            while ((it1 < tmp[0].size()) && (it2 < tmp[1].size())) {
                find_bigger(value, std::ref(it1), 0);
                find_less(value, std::ref(it2), 1);
                if ((it1 < tmp[0].size()) && (it2 < tmp[1].size())) {
                    swap(it1,it2);
                }
            }
            rewrite_file(beg+s1, it1, 0);
            rewrite_file(beg+l+1+s2, it2, 1);
            s1+=it1;
            s2+=it2;
            //std::cerr << "it1: " << it1 << " it2: " << it2 << " value: " << value << " val1: " << val1 << " val2: " << val2 << std::endl;
            //get_vector("in");

            for (size_t i = 0; i < file.size(); ++i) {
                fclose(file[i]);
                file[i] = fopen(file_name.c_str(), "rb+");
            }
        }
        sort(beg, beg+l);
        sort(beg+l+1, end);
    }
}

void fill_file_by_random(const std::string& file_name) {
    FILE* file = fopen(file_name.c_str(), "wb");
    int n = 10000;
    for (int i = 0; i < n; ++i) {
        int64_t x = rand() ;
        fwrite(&x, sizeof(x), 1, file);
    }
    fclose(file);
}

int main() {
    std::string file_name = "in";
    fill_file_by_random(file_name);
    auto initial_v = get_vector(file_name);
    sort(initial_v.begin(), initial_v.end());
    for (size_t i = 0; i < 2; ++i) {
        file[i] = fopen(file_name.c_str(), "rb+");
        tmp[i].reserve(block_size);
    }
    fseek(file[0], 0, 2);
    size_t n = ftell(file[0]) / sizeof(int64_t);
    std::cerr << n << std::endl;
    sort(0, n);
    auto final_v = get_vector(file_name);
    assert(initial_v == final_v);
    for (int i = 0; i < file.size(); ++i) {
        fclose(file[i]);
    }
    return 0;
}
