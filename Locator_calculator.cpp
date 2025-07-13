#include <iostream>
#include <cstdio>
#include <vector>
#include <string>

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
            int log_value = log_table[value] - log_table[other.get_value()] + 63;
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

        bool operator!=(const GF64& other) const {
            return value != other.value;
        }
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
        GF64_poly set_coefficients(int index, GF64 value) {
            if(index > degree){
                degree = index;
                coefficients.resize(degree + 1);
            }
            coefficients[index] = value;
            return *this;
        }
        int get_degree() const {
            return degree;
        }
        GF64_poly(const std::vector<GF64>& coefficients) {
            this->coefficients = coefficients;
            this->degree = coefficients.size() - 1;
            while(degree > 0 && coefficients[degree].get_value() == 0){
                degree--;
            }
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

        GF64 operator()(const GF64& x) const {
            if(x.get_value() == 0){
                return coefficients[0];
            }
            GF64 result(0);
            GF64 power(1);
            for(int i = 0; i <= degree; i++){
                result = result + coefficients[i] * power;
                power = power * x;
            }
            return result;
        }
        GF64_poly differentiate() const {
            GF64_poly result;
            result.set_coefficients(0, coefficients[1]);
            for(int i = 1; i < degree; i++){
                if(i % 2 == 0){
                    result.set_coefficients(i, coefficients[i+1]);
                }
                else result.set_coefficients(i, 0);
            }
            return result;
        }
        void print() const {
            for(int i = 0; i < coefficients.size(); i++){
                printf("%d ", coefficients[i].get_value());
            }
            printf("\n");
        }
        bool is_zero() const {
            return degree == 0 && coefficients[0].get_value() == 0;
        }
        GF64_poly mod_x21() const {
            GF64_poly result;
            for(int i = 20; i >= 0; i--){
                if(coefficients[i].get_value() != 0)
                    result.set_coefficients(i, coefficients[i]);
            }
            return result;
        }
};

class Locator_calculator{
    private:
        GF64_poly locator;
        GF64_poly erasure_locator;
        GF64_poly error_locator;
        GF64_poly error_and_erasures_locator;
        GF64_poly error_and_erasures_evaluator;

    public:
        Locator_calculator(){}

        GF64_poly calculateErasureLocator(const std::vector<bool>& erasures){
            GF64_poly erasure_locator(std::vector<GF64>{GF64(1)});
            for(int i = 0; i < 63; i++){
                if(erasures[i]){
                    erasure_locator = erasure_locator * GF64_poly(std::vector<GF64>{GF64(1), GF64(pow_table[(63 - i) % 63])});
                }
            }
            erasure_locator = erasure_locator.mod_x21();
            erasure_locator.print();
            return erasure_locator;
        }

        GF64_poly calculateErrorLocator(const std::vector<GF64>& received, const std::vector<GF64>& original, const std::vector<bool>& erasures){
            GF64_poly error_locator(std::vector<GF64>{GF64(1)});
            for(int i = 0; i < 63; i++){
                if(original[i] != received[i] && !erasures[i]){
                    error_locator = error_locator * GF64_poly(std::vector<GF64>{GF64(1), GF64(pow_table[(63 - i) % 63])});
                }
            }
            error_locator = error_locator.mod_x21();
            error_locator.print();
            return error_locator;
        }

        GF64_poly calculateErrorAndErasuresLocator(const std::vector<GF64>& received, const std::vector<bool>& erasures){
            error_and_erasures_locator = error_locator * erasure_locator;
            error_and_erasures_locator.print();
            return error_and_erasures_locator;
        }
};



int main(){
    Locator_calculator locator_calculator;
    std::vector<GF64> original(63);
    std::vector<GF64> received(63);
    std::vector<bool> erasures(63);
    for(int i = 0; i < 63; i++){
        int x;
        scanf("%d", &x);
        original[i] = GF64(x);
    }
    for(int i = 0; i < 63; i++) {
        char c; int x;
        scanf(" %c", &c);
        if(c == '*'){
            received[i] = GF64(0);
            erasures[i] = true;
        }
        else{
            ungetc(c, stdin);
            scanf("%d", &x);
            received[i] = GF64(x);
            erasures[i] = false;
        }
    }
    locator_calculator.calculateErasureLocator(erasures);
    locator_calculator.calculateErrorLocator(received, original, erasures);
    locator_calculator.calculateErrorAndErasuresLocator(received, erasures);
    
}