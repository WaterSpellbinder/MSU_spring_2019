#include <iostream>
#include <cstdlib>
#include <cassert>
#include <memory.h>
 
class BigInt {

protected:
    int *mem_ = nullptr;
    int capacity_;
    int size_;
 
    bool negative_;

public:
    BigInt(long long x = 0) {
        negative_ = (x < 0);
        if (x < 0) {
            x = -x;
        }
 
        allocate(20);   // 10^20 > LLONG_MAX
        if (x == 0) {
            size_= 1;
            mem_[0] = 0;
        } else {
            size_ = 0;
            while (x > 0) {
                mem_[size_] = x % 10;
                x /= 10;
                ++size_;
            }
        }
    }
 
    BigInt(const BigInt& num) {
        negative_ = num.negative_;
        allocate(num.size_);
        memcpy(mem_, num.mem_, num.size_ * sizeof(int));
        size_ = capacity_;
    }
 
    BigInt& operator =(const BigInt& num) {
        if (&num == this) {
            return *this;
        }
        negative_ = num.negative_;
        reallocate(num.size_);
        memcpy(mem_, num.mem_, num.size_ * sizeof(int));
        size_ = capacity_;
        return *this;
    }

    ~BigInt() {
        clear();
    }
 
    BigInt operator +(const BigInt& num) const {
        BigInt result(*this);
        return result += num;
    }
 
    BigInt operator -(const BigInt& num) const {
        BigInt result(*this);
        return result -= num;
    }
 
    BigInt operator -() const {
        BigInt result;
        if ( !(size_ == 1 && mem_[0] == 0) ) {
            result.negative_ = !negative_;
        } else {
            result.negative_=false;
        }
        result.reallocate(size_);
        memcpy(result.mem_, mem_, size_ * sizeof(int));
        result.size_=size_;
            
        return result;
    }
 
    BigInt& operator +=(const BigInt& num) {
        if (negative_ == num.negative_) {
            addAbsoluteValue(num);
        } else {
            if (lessByAbsoluteValue(num)) {
                negative_ = num.negative_;
                subtractAbsoluteValueFrom(num);
            } else {
                subtractAbsoluteValue(num);
            }
        }
        if ((size_ == 1) && (mem_[0] == 0))
            negative_ = false;
        return *this; 
    }
 
    BigInt& operator -=(const BigInt& num) {
        if (negative_ != num.negative_) {
            addAbsoluteValue(num);
        } else {
            if (lessByAbsoluteValue(num)) {
                negative_ = !negative_;
                subtractAbsoluteValueFrom(num);
            } else {
                subtractAbsoluteValue(num);
            }
        }
        if ((size_ == 1) && (mem_[0] == 0))
            negative_ = false;
        return *this; 
    }
 
    bool operator <(const BigInt& num) const {
        if (negative_ != num.negative_) {
            return negative_;
        } else if (equalByAbsoluteValue(num)) {
            return false;
        } else {
            return negative_ ^ lessByAbsoluteValue(num);
        }
    }
 
    bool operator ==(const BigInt& num) const {
        if (negative_ != num.negative_) {
            return false;
        }
        else if (equalByAbsoluteValue(num)) {
            return true;
        } else {
            return false;
        }
    }
 
    bool operator >(const BigInt& num) const {
        return num < *this;
    }
 
    bool operator <=(const BigInt& num) const {
        return !(*this > num);
    }
 
    bool operator >=(const BigInt& num) const {
        return !(*this < num);
    }
 
    bool operator !=(const BigInt& num) const {
        return !(*this == num);
    }
 
    int size() const {
        return size_;
    }
 
    const int* getMem() const {
        return mem_;
    }
 
private:
    void clear() {
        if (mem_) {
            free(mem_);
            mem_ = nullptr;
        }
        capacity_ = 0;
    }
 
    void allocate(int capacity) {
        assert(!mem_);
        capacity_ = capacity;
        mem_ = (int*)malloc(sizeof(int) * capacity_);
        memset(mem_, 0, sizeof(int) * capacity_);
    }
 
    void reallocate(int capacity) {
        capacity_ = capacity;
        mem_ = (int*)realloc(mem_, sizeof(int) * capacity);
    }
 
    bool equalByAbsoluteValue(const BigInt& num) const {
        if (size_ != num.size_) {
            return false;
        } 
        else {
            return memcmp(mem_, num.mem_, size_ * sizeof(int)) == 0;
        }
    }
 
    bool lessByAbsoluteValue(const BigInt& num) const {
        if (size_ != num.size_) {
            return size_ < num.size_;
        } else {
            for (int i = size_-1; i >= 0; --i) {
                if (mem_[i] != num.mem_[i]) {
                    return mem_[i] < num.mem_[i];
                }
            }
            return false;
        }
    }
 
    void addAbsoluteValue(const BigInt& other) {
        reallocate(std::max(size_, other.size_) + 1);
        int carry = 0;
        int old_size = size_;
        size_ = 0;
        while (carry > 0 || size_ < old_size || size_ < other.size_) {
            if (size_ < old_size) {
                std::cerr << mem_[size_] << " ";
                carry += mem_[size_];
            }
            if (size_ < other.size_) {
                std::cerr << other.mem_[size_] << " ";
                carry += other.mem_[size_];
            }
            std::cerr << ": " << carry << "\n";
            mem_[size_] = carry % 10;
            carry /= 10;
            ++size_;
        }
    }
 
    void subtractAbsoluteValue(const BigInt& other) {
        bool t = false;
        int s=0;
        for (int i = 0; i < std::max(size_, other.size_); ++i) {
            int a = 0, b = 0;
            if (i < size_) {
                a = mem_[i];
            }
            if (i < other.size_) {
                b = other.mem_[i];
            }
            if (a >= b) {
                if (t == false) {
                    mem_[i] = a-b;
                } else {
                    if (a > b) {
                        mem_[i] = a-1-b;
                        t = false;
                    } else {
                        mem_[i] = a+9-b;
                    }
                }
            } else {
                if (t == false) {
                    t = true;
                    mem_[i] = a+10-b;
                } else {
                    mem_[i] = a+9-b;
                }
            }
            if (mem_[i] > 0) s = i;
        }
        size_ = s + 1;
    }

    void subtractAbsoluteValueFrom(const BigInt& other) {
        bool t = false;
        int s=0;
        for (int i = 0; i < std::max(size_, other.size_); ++i) {
            int a = 0, b = 0;
            if (i < size_) {
                b = mem_[i];
            }
            if (i < other.size_) {
                a = other.mem_[i];
            }
            if (a >= b) {
                if (t == false) {
                    mem_[i] = a-b;
                } else {
                    if (a > b) {
                        mem_[i] = a-1-b;
                        t = false;
                    } else {
                        mem_[i] = a+9-b;
                    }
                }
            } else {
                if (t == false) {
                    t = true;
                    mem_[i] = a+10-b;
                } else {
                    mem_[i] = a+9-b;
                }
            }
            if (mem_[i] > 0) s = i;
        }
        size_ = s + 1;
    }
};
 
std::ostream& operator <<(std::ostream& ostream, const BigInt& num) {
    if (num.size() == 0) {
        ostream << "0";
        return ostream;
    }
    if (num < 0) {
        ostream << "-";
    }
    const int *mem = num.getMem();
    for (int i = num.size() - 1; i >= 0; --i) {
        ostream << mem[i];
    }
    return ostream;
}
