#include <iostream>
#include <vector>
#include <string>

// Power table for GF(64)
int pow_table[63] = {1, 2, 4, 8, 16, 32, 3, 6, 12, 24, 48, 35, 5, 10, 20, 40, 19, 38, 15,
                     30, 60, 59, 53, 41, 17, 34, 7, 14, 28, 56, 51, 37, 9, 18, 36, 11, 22, 44,
                     27, 54, 47, 29, 58, 55, 45, 25, 50, 39, 13, 26, 52, 43, 21, 42, 23, 46, 31, 
                     62, 63, 61, 57, 49, 33};

class GF64 {
    private:
        int value;
    public:
        GF64() { this->value = 0; }
        GF64(int value) { this->value = value; }
        int get_value() const { return value; }
        void set_value(int value) { this->value = value; }
};

// Calculate distance between two codewords
// Returns a pair of (total_distance, num_errors, num_erasures)
std::pair<int, std::pair<int, int>> calculate_distance(const std::vector<GF64>& word1, 
                                                      const std::vector<GF64>& word2) {
    int total_distance = 0;
    int num_errors = 0;
    int num_erasures = 0;
    
    for(int i = 0; i < 63; i++) {
        if(word1[i].get_value() == -1 || word2[i].get_value() == -1) {
            // If either position is an erasure
            total_distance += 1;
            num_erasures++;
        }
        else if(word1[i].get_value() != word2[i].get_value()) {
            // If values are different (error)
            total_distance += 2;
            num_errors++;
        }
    }
    
    return {total_distance, {num_errors, num_erasures}};
}

int main() {
    // Read first codeword
    std::vector<GF64> word1(63);
    std::cout << "Enter first codeword (63 values, use * for erasures):\n";
    for(int i = 0; i < 63; i++) {
        char c;
        std::cin >> c;
        if(c == '*') {
            word1[i] = GF64(-1);  // -1 represents erasure
        } else {
            std::cin.unget();
            int x;
            std::cin >> x;
            if(x < 0 || x > 63) {
                std::cout << "Invalid input: values must be between 0 and 63\n";
                return 1;
            }
            word1[i] = GF64(x);
        }
    }
    
    // Read second codeword
    std::vector<GF64> word2(63);
    std::cout << "Enter second codeword (63 values, use * for erasures):\n";
    for(int i = 0; i < 63; i++) {
        char c;
        std::cin >> c;
        if(c == '*') {
            word2[i] = GF64(-1);  // -1 represents erasure
        } else {
            std::cin.unget();
            int x;
            std::cin >> x;
            if(x < 0 || x > 63) {
                std::cout << "Invalid input: values must be between 0 and 63\n";
                return 1;
            }
            word2[i] = GF64(x);
        }
    }
    
    // Calculate and print distance
    auto [total_distance, counts] = calculate_distance(word1, word2);
    auto [num_errors, num_erasures] = counts;
    
    std::cout << "Distance calculation results:\n";
    std::cout << "Total distance: " << total_distance << "\n";
    std::cout << "Number of errors: " << num_errors << "\n";
    std::cout << "Number of erasures: " << num_erasures << "\n";
    
    return 0;
}
