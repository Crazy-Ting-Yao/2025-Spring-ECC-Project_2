#include <iostream>
#include <cstdio>
#include <vector>
// Power table for GF(64)
int pow_table[63] = {1, 2, 4, 8, 16, 32, 3, 6, 12, 24, 48, 35, 
                     5, 10, 20, 40, 19, 38, 15, 30, 60, 59, 53, 
                     41, 17, 34, 7, 14, 28, 56, 51, 37, 9, 18, 
                     36, 11, 22, 44, 27, 54, 47, 29, 58, 55, 45, 
                     25, 50, 39, 13, 26, 52, 43, 21, 42, 23, 46,
                     31, 62, 63, 61, 57, 49, 33};
// Logarithm table for GF(64)
int log_table[64];
// Coefficients of the generator polynomial for the Reed-Solomon code
const int gen_poly[22] = {58, 62, 59, 7, 35, 58, 63, 47, 51, 6, 33, 
                            43, 44, 27, 7, 53, 39, 62, 52, 41, 44, 1};
class GF64 {
    private:
        int value; 
    public:
        GF64() { this->value = 0; }
        GF64(int value) { this->value = value; }
        // Add the polynomial
        GF64 operator+(const GF64& other) const {
            // Simple XOR operation
            return GF64(value ^ other.value);
        }
        // Multiply the polynomial
        GF64 operator*(const GF64& other) const {
            // If one of the two numbers is 0, the result is 0
            if(value == 0 || other.get_value() == 0) return GF64(0);
            // Use the log table to times two GF64 numbers
            int log_value = log_table[value] + log_table[other.get_value()];
            return GF64(pow_table[log_value % 63]);
        }
        // Divide the polynomial
        GF64 operator/(const GF64& other) const {
            // If the other number is 0, the result is undefined
            if(other.get_value() == 0) 
                throw std::invalid_argument("Division by zero");
            // If the number is 0, the result is 0
            if(value == 0) return GF64(0);
            // Use the log table to divide two GF64 numbers
            int log_value = log_table[value] - log_table[other.get_value()] + 63;
            return GF64(pow_table[log_value % 63]);
        }
        // Assign the value of the GF64 number
        GF64 operator=(const GF64& other) {
            this->value = other.value;
            return *this;
        }
        // Get the value of the GF64 number
        int get_value() const {
            return value;
        }
};

class GF64_poly {
    private:
        // The coefficients of the polynomial (The first element is the constant term)
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
            while(degree > 0 && coefficients[degree].get_value() == 0) degree--;
        }
        // Set the coefficients of the polynomial
        GF64_poly set_coefficients(int index, GF64 value) {
            // If the index is greater than the degree, resize the polynomial
            if(index > degree){
                degree = index;
                coefficients.resize(degree + 1);
            }
            coefficients[index] = value;
            return *this;
        }
        // Get the degree of the polynomial
        int get_degree() const {
            return degree;
        }
        // Add two polynomials
        GF64_poly operator+(const GF64_poly& other) const {
            // Initialize the result with zeros, the result's degree is the maximum degree of the two polynomials
            std::vector<GF64> result(std::max(degree, other.degree) + 1, GF64(0));
            for(int i = 0; i <= degree; i++) result[i] = coefficients[i];
            for(int i = 0; i <= other.degree; i++) result[i] = result[i] + other.coefficients[i];
            // Remove leading zeros while keeping at least one term
            while(result.size() > 1 && result.back().get_value() == 0) result.pop_back();
            return GF64_poly(result);
        }
        // Multiply a polynomial and a GF64 number
        GF64_poly operator*(const GF64& other) const {
            // Initialize the result with zeros, the result's degree is the degree of the polynomial
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
        // Multiply two polynomials
        GF64_poly operator*(const GF64_poly& other) const {
            // Initialize result with zeros, the result's degree is the sum of the degrees of the two polynomials
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
            while(result.size() > 1 && result.back().get_value() == 0) result.pop_back();
            return GF64_poly(result);
        }
        // Divide a polynomial by another polynomial
        GF64_poly operator/(const GF64_poly& other) const {
            // Check for division by zero polynomial
            if(other.degree == 0 && other.coefficients[0].get_value() == 0) {
                std::cout << "Error: Division by zero polynomial" << std::endl;
                exit(1);
            }
            // If the degree of the polynomial is less than the degree of the other polynomial, the result is 0
            if(degree < other.degree) {
                return GF64_poly();
            }
            // Initialize the quotient with zeros
            // The result's degree is the degree of the polynomial minus the degree of the other polynomial
            std::vector<GF64> quotient(degree - other.degree + 1, GF64(0));
            std::vector<GF64> remainder = coefficients;
            // Perform polynomial division
            for(int i = degree; i >= other.degree; i--) {
                // If the leading coefficient of the remainder is not 0
                if(remainder[i].get_value() != 0) {
                    GF64 coef = remainder[i] / other.coefficients[other.degree];
                    quotient[i - other.degree] = coef;
                    // Update the remainder
                    // Y = X * Q + R
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
            // Remove leading zeros while keeping at least one term
            while(degree > 0 && coefficients[degree].get_value() == 0) {
                degree--;
            }
            return *this;
        }
        // Evaluate the polynomial at a given number
        GF64 operator()(const GF64& x) const {
            // If the number is 0, the result is the constant term
            if(x.get_value() == 0){
                return coefficients[0];
            }
            GF64 result(0), power(1);
            for(int i = 0; i <= degree; i++){
                result = result + coefficients[i] * power;
                // Update the power (a^i)
                power = power * x;
            }
            return result;
        }
        GF64_poly differentiate() const {
            GF64_poly result;
            // If the degree is even, the result is the coefficient of the polynomial
            // Otherwise, the result is 0 (1+1=0 in GF(64))
            for(int i = 0; i < degree; i++){
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
            // The result is simply the polynomial dropping all the terms with degree greater than 20
            GF64_poly result;
            for(int i = 20; i >= 0; i--){
                if(coefficients[i].get_value() != 0)
                    result.set_coefficients(i, coefficients[i]);
            }
            return result;
        }
};

class ReedSolomonDecoder {
    private:
        static const int n = 63;  // Code length
        static const int k = 42;  // Message length
        static const int t = 10;  // Error correction capability
    
    
    // Calculate syndromes including erasure information
    GF64_poly calculateSyndromes(const std::vector<GF64>& received, 
                                 const std::vector<bool>& erasures) {
        std::vector<GF64> syndromes;
        syndromes.resize(21);
        for (int j = 0; j <= 20; j++) {
            // Syndrome S_j = sum(a^ij * c_i), j = 1~21
            GF64 syndrome(0);
            for (int i = 0; i < n; i++) {
                if (received[i].get_value() != 0) {
                    GF64 alpha(pow_table[(i * (j+1)) % 63]);
                    syndrome = syndrome + received[i] * alpha;
                }
            }
            // Save the syndrome with shifted ( syndromes[j] = s_(j+1) )
            syndromes[j] = syndrome;
        }
        // Return the syndrome polynomial
        return GF64_poly(syndromes);
    }

    // Calculate erasure locator polynomial
    GF64_poly calculateErasureLocator(const std::vector<bool>& erasures) {
        // Initialize the erasure locator polynomial
        GF64_poly erasureLocator(std::vector<GF64>{GF64(1)});
        for (int i = 0; i < n; i++) {
            if (erasures[i]) {
                // Multiply by (1 + a^i * x)
                std::vector<GF64> factor = {GF64(1), GF64(pow_table[i])};
                GF64_poly factorPoly(factor);
                erasureLocator = erasureLocator * factorPoly;
            }
        }
        // If the degree of the erasure locator polynomial is greater than 21, the decoding fails
        if(erasureLocator.get_degree() > 21){
            std::cout << "Error: Erasure locator polynomial degree exceeds 21" << std::endl;
            exit(1);
        }
        return erasureLocator;
    }

    // Euclidean algorithm
    std::pair<GF64_poly, GF64_poly> euclideanAlgorithm(
        const GF64_poly& syndromes,
        const GF64_poly& erasureLocator) {
        
        // Initialize polynomials
        GF64_poly S_0(erasureLocator * syndromes); // Modified Syndrome Polynomial
        // mod x^21
        GF64_poly x21(std::vector<GF64>{GF64(0), GF64(0), GF64(0), GF64(0), 
        GF64(0), GF64(0), GF64(0), GF64(0), GF64(0), GF64(0), GF64(0), GF64(0), 
        GF64(0), GF64(0), GF64(0), GF64(0), GF64(0), GF64(0), GF64(0), GF64(0), 
        GF64(0), GF64(1)});

        S_0 = S_0.mod_x21();
        int num_of_erasures = erasureLocator.get_degree();
        // mu = lower bound of (r-e_0)/2
        int mu = (21 - num_of_erasures) / 2;
        // nu = upper bound of (r+e_0)/2 - 1
        int nu = (21 + num_of_erasures + 1) / 2 - 1; 
        // Initialize the polynomials
        std::vector<GF64_poly> R, Q, U, V;
        // R_0 = x^r = x^21
        R.push_back(x21);
        R.push_back(S_0);
        // Q_0 = 0 (Don't care)
        Q.push_back(GF64_poly(std::vector<GF64>{0}));
        // Q_1 = 0 (Don't care)
        Q.push_back(GF64_poly(std::vector<GF64>{0}));
        // S_0 = 1 
        U.push_back(GF64_poly(std::vector<GF64>{1}));
        // S_1 = 0
        U.push_back(GF64_poly(std::vector<GF64>{0}));
        // T_0 = 0
        V.push_back(GF64_poly(std::vector<GF64>{0}));
        // T_1 = 1
        V.push_back(GF64_poly(std::vector<GF64>{1}));
        while(R[R.size() - 1].get_degree() > nu || V[V.size() - 1].get_degree() > mu){
            // Q_i = R_(i-2) / R_(i-1)
            GF64_poly Q_i = R[R.size() - 2] / R[R.size() - 1];
            Q.push_back(Q_i);
            // R_i = R_(i-2) + R_(i-1) * Q_i
            R.push_back(R[R.size() - 2] + R[R.size() - 1] * Q_i);
            // U_i = U_(i-2) + U_(i-1) * Q_i
            U.push_back(U[U.size() - 2] + U[U.size() - 1] * Q_i);
            // V_i = V_(i-2) + V_(i-1) * Q_i
            V.push_back(V[V.size() - 2] + V[V.size() - 1] * Q_i);
        }
        // Return the error locator and error evaluator
        return std::make_pair(V[V.size() - 1], R[R.size() - 1]);
    }

    // Error correction
    std::pair<bool, GF64_poly> correctErrors(
        GF64_poly& erasureLocator,
        GF64_poly& errorLocator,
        GF64_poly& error_and_erasure_Evaluator
    )
    {
        // Initialize the error locator polynomial
        GF64_poly error_and_erasures_Locator = errorLocator * erasureLocator;
        bool is_correctable = false;
        std::vector<GF64> err(n);
        // Time domain completion
        // If the error locator polynomial is 0, the decoding fails
        if(error_and_erasures_Locator(0).get_value() == 0) {
            return std::make_pair(false, err);
        }
        // deg(w) < e_0 + deg(erasureLocator)
        if(error_and_erasure_Evaluator.get_degree() >= errorLocator.get_degree() + erasureLocator.get_degree()) {
            return std::make_pair(false, err);
        }
        int count = 0;
        // Get the formal derivative of the error locator polynomial
        GF64_poly error_and_erasures_Locator_derivative = error_and_erasures_Locator.differentiate();
        // Error value
        for(int i = 0; i < n; i++){
            GF64 alpha = pow_table[(63 - i) % 63];
            if(error_and_erasures_Locator(alpha).get_value() == 0 && error_and_erasures_Locator_derivative(alpha).get_value() != 0){
                count++;
                err[i] = error_and_erasure_Evaluator(alpha) / error_and_erasures_Locator_derivative(alpha);
            }
            else{
                err[i] = GF64(0);
            }
        }
        // If the number of error is equal to the degree of the error locator polynomial, the error is correctable
        is_correctable = (count == error_and_erasures_Locator.get_degree());
        return std::make_pair(is_correctable, GF64_poly(err));
    }

public:
    std::pair<bool, GF64_poly> decode(const std::vector<GF64>& received, 
                            const std::vector<bool>& erasures = std::vector<bool>()) {
        GF64_poly received_poly(received);
        // Calculate syndromes
        GF64_poly syndromes = calculateSyndromes(received, erasures);
        if(syndromes.is_zero()){
            return std::make_pair(true, received_poly);
        }
        // Calculate erasure locator polynomial
        GF64_poly erasureLocator = calculateErasureLocator(erasures);
        // Apply the Euclidean algorithm, return error locator and error evaluator
        std::pair<GF64_poly, GF64_poly> result = euclideanAlgorithm(syndromes, erasureLocator);
        // Error correction
        std::pair<bool, GF64_poly> error_correction_result = correctErrors(erasureLocator, result.first, result.second);
        // codeword = received + error
        GF64_poly codeword = received_poly + error_correction_result.second;
        // Return the result
        return std::make_pair(error_correction_result.first, codeword);
    }
};


int main() {
    log_table[0] = 0;
    for(int i = 1; i < 64; i++) {
        log_table[pow_table[i-1]] = i-1;
    }
    std::vector<GF64> received(63);
    std::vector<bool> erasures(63);
    for(int i = 0; i < 63; i++) {
        char c; int x;
        scanf(" %c", &c);
        if(c == '*'){
            // If the received codeword is an erasure, set the erasure to true
            received[i] = GF64(0);
            erasures[i] = true;
        }
        else{
            // If the received codeword is not an erasure, set the erasure to false
            ungetc(c, stdin);
            scanf("%d", &x);
            received[i] = GF64(x);
            erasures[i] = false;
        }
    }
    
    ReedSolomonDecoder decoder;
    // Decode the received codeword
    std::pair<bool, GF64_poly> decoded = decoder.decode(received, erasures);
    if(decoded.first){
        // Print the decoded codeword
        decoded.second.print();
    }
    else{
        // If the decoding fails, print "give up"
        printf("give up\n");
    }
    
    return 0;
}

