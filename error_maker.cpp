#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>

// Power table for GF(64)
int pow_table[63] = {1, 2, 4, 8, 16, 32, 3, 6, 12, 24, 48, 35, 5, 10, 20, 40, 19, 38, 15,
                     30, 60, 59, 53, 41, 17, 34, 7, 14, 28, 56, 51, 37, 9, 18, 36, 11, 22, 44,
                     27, 54, 47, 29, 58, 55, 45, 25, 50, 39, 13, 26, 52, 43, 21, 42, 23, 46, 31, 
                     62, 63, 61, 57, 49, 33};
// Logarithm table for GF(64)
int log_table[64];

// Include the same GF64 class and tables from proj2.cpp
class GF64 {
    private:
        int value;
    public:
        GF64() { this->value = 0; }
        GF64(int value) { this->value = value; }
        GF64 operator+(const GF64& other) const {
            return GF64(value ^ other.value);
        }
        GF64 operator*(const GF64& other) const {
            if(value == 0 || other.get_value() == 0) {
                return GF64(0);
            }
            int log_value = log_table[value] + log_table[other.get_value()];
            return GF64(pow_table[log_value % 63]);
        }
        int get_value() const { return value; }
        void set_value(int value) { this->value = value; }
};

void initialize_tables() {
    log_table[0] = 0;
    for(int i = 1; i < 64; i++) {
        log_table[pow_table[i-1]] = i-1;
    }
}

std::vector<GF64> generate_corrupted_codeword(const std::vector<GF64>& original, 
                                            int num_errors, 
                                            int num_erasures) {
    std::vector<GF64> corrupted = original;
    std::vector<bool> used_positions(63, false);
    
    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 62);
    
    // Generate erasures
    for(int i = 0; i < num_erasures; i++) {
        int pos;
        do {
            pos = dis(gen);
        } while(used_positions[pos]);
        
        used_positions[pos] = true;
        corrupted[pos] = GF64(-1);  // Use -1 to mark erasures
    }
    
    // Generate errors
    for(int i = 0; i < num_errors; i++) {
        int pos;
        do {
            pos = dis(gen);
        } while(used_positions[pos]);
        
        used_positions[pos] = true;
        // Generate random non-zero error value
        int error_value;
        do {
            error_value = dis(gen) + 1;  // +1 to avoid 0
        } while(error_value > 63);
        
        corrupted[pos] = corrupted[pos] + GF64(error_value);
    }
    
    return corrupted;
}

int main() {
    initialize_tables();
    
    // Read original codeword
    std::vector<GF64> original(63);
    std::cout << "Enter the original codeword (63 values, 0-63):\n";
    for(int i = 0; i < 63; i++) {
        int x;
        std::cin >> x;
        if(x < 0 || x > 63) {
            std::cout << "Invalid input: values must be between 0 and 63\n";
            return 1;
        }
        original[i] = GF64(x);
    }
    
    // Read number of errors and erasures
    int num_errors, num_erasures;
    std::cout << "Enter number of errors: ";
    std::cin >> num_errors;
    std::cout << "Enter number of erasures: ";
    std::cin >> num_erasures;
    
    // Generate corrupted codeword
    std::vector<GF64> corrupted = generate_corrupted_codeword(original, num_errors, num_erasures);
    
    // Output the corrupted codeword
    std::cout << "Corrupted codeword:\n";
    for(int i = 0; i < 63; i++) {
        if(corrupted[i].get_value() == -1) {
            std::cout << "* ";  // Print * for erasures
        } else {
            std::cout << corrupted[i].get_value() << " ";
        }
    }
    std::cout << "\n";
    
    return 0;
}

/*
58 4 63 56 27 33  * 49 2 4  * 14 34 57  * 38 44 57 38 15 15 34 34 34 34 34 34 34 38 34  * 34  * 34 34  *  * 34 34 34  * 34 24 38 29 26 57 * 60 19 32 *  7 11 * 27 28 41 14 48 4 * * 
58 4 63 56 27 33 30 49 2 4 37 14 34 57 62 11 44 18 38 15 35 34 34 34 34 34 34 34 34 34 34 34 34 34 34 34 34 34 34 34 34 34 24 38 29 26 57 3 60 19 32 38 7 44 0 27 28 41 14 48 4 45 1 
*/