#include <iostream>
#include <vector>
#include <string>

// Power table for GF(64)
int pow_table[63] = {1, 2, 4, 8, 16, 32, 3, 6, 12, 24, 48, 35, 5, 10, 20, 40, 19, 38, 15,
                     30, 60, 59, 53, 41, 17, 34, 7, 14, 28, 56, 51, 37, 9, 18, 36, 11, 22, 44,
                     27, 54, 47, 29, 58, 55, 45, 25, 50, 39, 13, 26, 52, 43, 21, 42, 23, 46, 31, 
                     62, 63, 61, 57, 49, 33};
// Logarithm table for GF(64)
int log_table[64];

// Generator polynomial coefficients
const int gen_poly[22] = {58, 62, 59, 7, 35, 58, 63, 47, 51, 6, 33, 43, 44, 27, 7, 53, 39, 62, 52, 41, 44, 1};

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
        GF64 operator/(const GF64& other) const {
            if(other.get_value() == 0) {
                throw std::runtime_error("Division by zero");
            }
            if(value == 0) {
                return GF64(0);
            }
            int log_value = log_table[value] - log_table[other.get_value()] + 63;
            return GF64(pow_table[log_value % 63]);
        }
        int get_value() const { return value; }
        void set_value(int value) { this->value = value; }
};

class GF64_poly {
    private:
        std::vector<GF64> coefficients;
        int degree;
    public:
        GF64_poly() {
            degree = 0;
            coefficients.resize(1);
            coefficients[0] = GF64(0);
        }
        GF64_poly(const std::vector<GF64>& coefficients) {
            this->coefficients = coefficients;
            this->degree = coefficients.size() - 1;
            while(degree > 0 && coefficients[degree].get_value() == 0){
                degree--;
            }
        }
        
        GF64_poly operator/(const GF64_poly& other) const {
            if(other.degree == 0 && other.coefficients[0].get_value() == 0) {
                throw std::runtime_error("Division by zero polynomial");
            }
            
            if(degree < other.degree) {
                return GF64_poly();
            }
            
            std::vector<GF64> quotient(degree - other.degree + 1, GF64(0));
            std::vector<GF64> remainder = coefficients;
            
            for(int i = degree; i >= other.degree; i--) {
                if(remainder[i].get_value() != 0) {
                    GF64 coef = remainder[i] / other.coefficients[other.degree];
                    quotient[i - other.degree] = coef;
                    
                    for(int j = 0; j <= other.degree; j++) {
                        remainder[i - j] = remainder[i - j] + 
                            other.coefficients[other.degree - j] * coef;
                    }
                }
            }
            
            while(quotient.size() > 1 && quotient.back().get_value() == 0) {
                quotient.pop_back();
            }
            return GF64_poly(quotient);
        }
        
        GF64_poly operator%(const GF64_poly& other) const {
            if(other.degree == 0 && other.coefficients[0].get_value() == 0) {
                throw std::runtime_error("Modulo by zero polynomial");
            }
            
            if(degree < other.degree) {
                return *this;
            }
            
            std::vector<GF64> remainder = coefficients;
            
            for(int i = degree; i >= other.degree; i--) {
                if(remainder[i].get_value() != 0) {
                    GF64 coef = remainder[i] / other.coefficients[other.degree];
                    
                    for(int j = 0; j <= other.degree; j++) {
                        remainder[i - j] = remainder[i - j] + 
                            other.coefficients[other.degree - j] * coef;
                    }
                }
            }
            
            while(remainder.size() > 1 && remainder.back().get_value() == 0) {
                remainder.pop_back();
            }
            return GF64_poly(remainder);
        }
        
        bool is_zero() const {
            return degree == 0 && coefficients[0].get_value() == 0;
        }
        
        void print() const {
            for(int i = 0; i < coefficients.size(); i++){
                printf("%d ", coefficients[i].get_value());
            }
            printf("\n");
        }
};

void initialize_tables() {
    log_table[0] = 0;
    for(int i = 1; i < 64; i++) {
        log_table[pow_table[i-1]] = i-1;
    }
}

bool verify_codeword(const std::vector<GF64>& codeword) {
    // Create generator polynomial
    std::vector<GF64> gen_poly_coeffs(gen_poly, gen_poly + 22);
    GF64_poly generator(gen_poly_coeffs);
    
    // Create codeword polynomial
    GF64_poly codeword_poly(codeword);
    
    // Divide codeword by generator polynomial
    GF64_poly remainder = codeword_poly % generator;
    
    // Check if remainder is zero
    return remainder.is_zero();
}

int main() {
    initialize_tables();
    
    // Read codeword
    std::vector<GF64> codeword(63);
    std::cout << "Enter the codeword (63 values, use * for erasures):\n";
    for(int i = 0; i < 63; i++) {
        char c;
        std::cin >> c;
        if(c == '*') {
            codeword[i] = GF64(-1);  // -1 represents erasure
        } else {
            std::cin.unget();
            int x;
            std::cin >> x;
            if(x < 0 || x > 63) {
                std::cout << "Invalid input: values must be between 0 and 63\n";
                return 1;
            }
            codeword[i] = GF64(x);
        }
    }
    
    // Verify codeword
    bool is_valid = verify_codeword(codeword);
    
    if(is_valid) {
        std::cout << "The codeword is valid (remainder is zero)\n";
    } else {
        std::cout << "The codeword is invalid (remainder is non-zero)\n";
    }
    
    return 0;
}
