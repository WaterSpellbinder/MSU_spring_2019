#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <iostream>
#include <array>
#include <vector>
#include <cassert>
#include <algorithm>

struct partition_elements {
    int64_t value;
    size_t l;
};

class SortBinaryFile {
public:
    void sort(const std::string& input_file_name) {
        file_name = input_file_name;
        for (size_t i = 0; i < 2; ++i) {
            file[i] = fopen(file_name.c_str(), "rb+");
            tmp[i].reserve(block_size);
        }
        fseek(file[0], 0, 2);
        size_t n = ftell(file[0]) / sizeof(int64_t);
        sort(0, n);
        for (size_t i = 0; i < file.size(); ++i) {
            fclose(file[i]);
        }
    }

private:
    void read_to_vec(size_t pos, int thread_num) {
        size_t n = tmp[thread_num].size();
        fseek(file[thread_num], pos * sizeof(int64_t), 0);
        fread(&tmp[thread_num][0], sizeof(int64_t), n, file[thread_num]);
    }

    void rewrite_file(size_t pos, size_t n, int thread_num) {
        fseek(file[thread_num], pos * sizeof(int64_t), 0);
        fwrite(&tmp[thread_num][0], sizeof(int64_t), n, file[thread_num]);
    }

    void cnt_less(int64_t value, size_t beg, size_t end, size_t &cnt, int thread_num) {
        fseek(file[thread_num], beg * sizeof(int64_t), 0);
        for (size_t i = beg; i < end; ++i) {
            int64_t val;
            fread(&val, sizeof(int64_t), 1, file[thread_num]);
            if (val <= value) {
                ++cnt;
            }
        }
    }

    void dump_file() {
        for (size_t i = 0; i < file.size(); ++i) {
            fclose(file[i]);
            file[i] = fopen(file_name.c_str(), "rb+");
        }
    }

    void swap_in_file(int64_t value1, int64_t value2, size_t place1, size_t place2, int thread_num) {
        fseek(file[thread_num], place1 * sizeof(int64_t), 0);
        fwrite(&value2, sizeof(int64_t), 1, file[thread_num]);
        fseek(file[thread_num], place2 * sizeof(int64_t), 0);
        fwrite(&value1, sizeof(int64_t), 1, file[thread_num]);
        dump_file();
    }

    void swap(size_t pos1, size_t pos2) {
        int64_t elem = tmp[0][pos1];
        tmp[0][pos1] = tmp[1][pos2];
        tmp[1][pos2] = elem;
    }

    void find_bigger(int64_t value, size_t& it, int thread_num) {
        while (it < tmp[thread_num].size()) {
            if (tmp[thread_num][it] > value) {
                break;
            } else {
                ++it;
            }
        }
    }

    void find_less(int64_t value, size_t& it, int thread_num) {
        while (it < tmp[thread_num].size()) {
            if (tmp[thread_num][it] <= value) {
                break;
            } else {
                ++it;
            }
        }
    }

    partition_elements Partition_function (size_t beg, size_t end) {
        size_t d = rand() % (end - beg);
        fseek(file[0], (beg + d) * sizeof(int64_t), 0);
        int64_t value;
        fread(&value, sizeof(int64_t), 1, file[0]);
        size_t l1 = 0, l2 = 0;
        std::thread thread1 (&SortBinaryFile::cnt_less, this, value, beg, beg + (end - beg) / 2, std::ref(l1), 0);
        std::thread thread2 (&SortBinaryFile::cnt_less, this, value, beg + (end - beg) / 2, end, std::ref(l2), 1);
        thread1.join();
        thread2.join();
        size_t l = l1 + l2 - 1;
        int64_t replaced;
        fseek(file[0], (beg + l) * sizeof(int64_t), 0);
        fread(&replaced, sizeof(int64_t), 1, file[0]);
        swap_in_file(value, replaced, beg + d, beg + l, 0);
        partition_elements p;
        p.value = value;
        p.l = l;
        return p;
    }

    void sort (size_t beg, size_t end) {
        if (end - beg <= block_size) {
            tmp[0].resize(end - beg);
            read_to_vec(beg, 0);
            std::sort(tmp[0].begin(), tmp[0].end());
            rewrite_file(beg, end - beg, 0);
            dump_file();
        } else {
            partition_elements p;
            p = Partition_function(beg, end);
            int64_t value = p.value;
            size_t l = p.l;
            size_t left_readed = 0, right_readed = 0;
            size_t r = (end - beg) - l - 1;
            while ((left_readed < l) && (right_readed < r)) {
                size_t it1 = 0, it2 = 0;
                tmp[0].resize(std::min(block_size, l - left_readed));
                tmp[1].resize(std::min(block_size, r - right_readed));
                std::thread thread1 (&SortBinaryFile::read_to_vec, this, beg + left_readed, 0);
                std::thread thread2 (&SortBinaryFile::read_to_vec, this, beg + l + 1 + right_readed, 1);
                thread1.join();
                thread2.join();
                while ((it1 < tmp[0].size()) && (it2 < tmp[1].size())) {
                    find_bigger(value, std::ref(it1), 0);
                    find_less(value, std::ref(it2), 1);
                    if ((it1 < tmp[0].size()) && (it2 < tmp[1].size())) {
                        swap(it1, it2);
                    }
                }
                rewrite_file(beg + left_readed, it1, 0);
                rewrite_file(beg + l + 1 + right_readed, it2, 1);
                left_readed += it1;
                right_readed += it2;
            }
            dump_file();
            sort(beg, beg + l);
            sort(beg + l + 1, end);
        }
    }

    std::array<FILE*, 2> file;
    const size_t block_size = 300000;
    std::array<std::vector<int64_t>, 2> tmp;
    std::string file_name;
};

std::vector<int64_t> get_vector(const std::string& file_name) {
    FILE* file = fopen(file_name.c_str(), "rb");
    fseek(file, 0, 2);
    size_t n = ftell(file) / sizeof(int64_t);
    fseek(file, 0, 0);
    std::vector<int64_t> v(n);
    fread(&v[0], sizeof(v[0]), n, file);
    fclose(file);
    return v;
}

void fill_file_by_random(const std::string& file_name) {
    FILE* file = fopen(file_name.c_str(), "wb");
    int n = 2000000;
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
    SortBinaryFile sorter;
    sorter.sort(file_name);
    auto final_v = get_vector(file_name);
    assert(initial_v == final_v);
    return 0;
}
