#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>


class GF64;
// Power table for GF(64)
int pow_table[63] = {1, 2, 4, 8, 16, 32, 3, 6, 12, 24, 48, 35, 5, 10, 20, 40, 19, 38, 15,
                     30, 60, 59, 53, 41, 17, 34, 7, 14, 28, 56, 51, 37, 9, 18, 36, 11, 22, 44,
                     27, 54, 47, 29, 58, 55, 45, 25, 50, 39, 13, 26, 52, 43, 21, 42, 23, 46, 31, 
                     62, 63, 61, 57, 49, 33};
// Logarithm table for GF(64)
int log_table[64];
// Coefficients of the generator polynomial for the Reed-Solomon code
const int gen_poly[22] = {58, 62, 59, 7, 35, 58, 63, 47, 51, 6, 33, 43, 44, 27, 7, 53, 39, 62, 52, 41, 44, 1};

class GF64 {
    private:
        int value; // Saves the coefficient of the polynomial
    public:
        // Constructor
        GF64() {
            this->value = 0;
        }
        GF64(int value) {
            this->value = value;
        }
        // Add the polynomial
        GF64 operator+(const GF64& other) const {
            // Simple XOR operation
            return GF64(value ^ other.value);
        }
        // Multiply the polynomial
        GF64 operator*(const GF64& other) const {
            // Use the log table to find the power of the polynomial
            if(value == 0 || other.get_value() == 0) {
                return GF64(0);
            }
            int log_value = log_table[value] + log_table[other.get_value()];
            return GF64(pow_table[log_value % 63]);
        }
        // Divide the polynomial
        GF64 operator/(const GF64& other) const {
            // Use the log table to find the power of the polynomial
            int log_value = log_table[value] - log_table[other.get_value()];
            return GF64(pow_table[log_value % 63]);
        }
        // Assign the value of the polynomial
        GF64 operator=(const GF64& other) {
            this->value = other.value;
            return *this;
        }
        // Get the value of the polynomial
        int get_value() const {
            return value;
        }
        // Set the value of the polynomial
        void set_value(int value) {
            this->value = value;
        }
};

class GF64_poly {
    public:
        std::vector<GF64> coefficients;
        int degree;
        
        GF64_poly(int degree) {
            coefficients.resize(degree + 1);
            for(int i = 0; i < coefficients.size(); i++) {
                coefficients[i] = GF64(0);
            }
            this->degree = degree;
        }
        
        GF64_poly(const std::vector<GF64>& coefficients) {
            this->coefficients = coefficients;
            this->degree = coefficients.size() - 1;
        }
        
        GF64_poly operator+(const GF64_poly& other) const {
            std::vector<GF64> result(std::max(degree, other.degree) + 1, GF64(0));
            for(int i = 0; i <= degree; i++) {
                result[i] = coefficients[i];
            }
            for(int i = 0; i <= other.degree; i++) {
                result[i] = result[i] + other.coefficients[i];
            }
            
            // Remove leading zeros while keeping at least one term
            while(result.size() > 1 && result.back().get_value() == 0) {
                result.pop_back();
            }
            return GF64_poly(result);
        }
        
        GF64_poly operator*(const GF64_poly& other) const {
            // Initialize result with zeros
            std::vector<GF64> result(degree + other.degree + 1, GF64(0));
            
            // Perform polynomial multiplication
            for(int i = 0; i <= degree; i++) {
                if(coefficients[i].get_value() != 0) {  // Skip zero terms
                    for(int j = 0; j <= other.degree; j++) {
                        if(other.coefficients[j].get_value() != 0) {  // Skip zero terms
                            result[i+j] = result[i+j] + coefficients[i] * other.coefficients[j];
                        }
                    }
                }
            }
            
            // Remove leading zeros while keeping at least one term
            while(result.size() > 1 && result.back().get_value() == 0) {
                result.pop_back();
            }
            return GF64_poly(result);
        }
        
        GF64_poly operator*(const GF64& other) const {
            std::vector<GF64> result(degree + 1);
            for(int i = 0; i <= degree; i++) {
                result[i] = coefficients[i] * other;
            }
            
            // Remove leading zeros while keeping at least one term
            while(result.size() > 1 && result.back().get_value() == 0) {
                result.pop_back();
            }
            return GF64_poly(result);
        }
        
        GF64_poly operator/(const GF64_poly& other) const {
            // Check for division by zero polynomial
            if(other.degree == 0 && other.coefficients[0].get_value() == 0) {
                throw std::invalid_argument("Division by zero polynomial");
            }
            
            if(degree < other.degree) {
                return GF64_poly(0);
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
            
            // Remove leading zeros while keeping at least one term
            while(quotient.size() > 1 && quotient.back().get_value() == 0) {
                quotient.pop_back();
            }
            return GF64_poly(quotient);
        }
        
        GF64_poly operator=(const GF64_poly& other) {
            this->coefficients = other.coefficients;
            this->degree = other.degree;
            return *this;
        }
        
        GF64_poly operator=(const std::vector<GF64>& coefficients) {
            this->coefficients = coefficients;
            this->degree = coefficients.size() - 1;
            return *this;
        }
};


class ReedSolomonEncoder {
private:
    static const int n = 63;  // Code length
    static const int k = 42;  // Message length
    static const int t = 10;  // Error correction capability

public:
    // Create generator polynomial
    GF64_poly createGeneratorPolynomial() {
        std::vector<GF64> gen_coeffs(22);
        for(int i = 0; i < 22; i++) {
            gen_coeffs[i] = GF64(gen_poly[i]);
        }
        return GF64_poly(gen_coeffs);
    }
    // Encode a message into a codeword
    
    std::vector<GF64> encode(const std::vector<GF64>& message) {
        if (message.size() != k) {
            throw std::invalid_argument("Message length must be " + std::to_string(k));
        }
        
        // Create message polynomial
        GF64_poly message_poly(message);
        
        // Create generator polynomial
        GF64_poly gen_poly = createGeneratorPolynomial();
        
        // Multiply polynomials
        GF64_poly codeword_poly = message_poly * gen_poly;
        
        // Convert to vector and ensure length is n
        std::vector<GF64> codeword = codeword_poly.coefficients;
        codeword.resize(n, GF64(0));  // Pad with zeros if necessary
        
        return codeword;
    }

    // Encode a message from raw integers
    std::vector<GF64> encodeFromInts(const std::vector<int>& message) {
        if (message.size() != k) {
            throw std::invalid_argument("Message length must be " + std::to_string(k));
        }

        std::vector<GF64> gfMessage;
        for (int value : message) {
            gfMessage.push_back(GF64(value));
        }
        
        return encode(gfMessage);
    }
};

// Example usage
int main() {
    // Initialize logarithm table
    srand(time(0));
    log_table[0] = 0;
    for(int i = 1; i < 64; i++) {
        log_table[pow_table[i-1]] = i-1;
    }

    // Create encoder
    ReedSolomonEncoder encoder;

    // Example message (42 symbols)
    // Randomly generate 42 symbols
    std::vector<int> message(42);
    for(int i = 0; i < 42; i++) {
        message[i] = rand() % 64;
    }

    // Encode the message
    std::vector<GF64> codeword = encoder.encodeFromInts(message);

    // Output the complete codeword
    for (const auto& symbol : codeword) {
        std::cout << symbol.get_value() << " ";
    }
    std::cout << std::endl;

    return 0;
}
