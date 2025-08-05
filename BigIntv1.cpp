#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <random> // For random number generation
#include <chrono> // For seeding random number generator
#include <iomanip> // For std::setw

// Constants
constexpr int MAX_DIGITS = 618; // Max decimal digits (e.g., for 2048-bit binary, roughly 617 decimal digits)
constexpr int HEX_SIZE = 128; // Max hexadecimal digits for BigHexInt (e.g., 512-bit number)
constexpr int KARATSUBA_THRESHOLD = 8; // Threshold for switching to naive multiplication in Karatsuba

// Custom Exception Classes
class BigIntException : public std::exception {
protected:
    std::string message;
public:
    BigIntException(const std::string& msg) : message(msg) {}
    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

class DivisionByZeroException : public BigIntException {
public:
    DivisionByZeroException() : BigIntException("Division by zero is not allowed") {}
};

class InvalidInputException : public BigIntException {
public:
    InvalidInputException(const std::string& input)
        : BigIntException("Invalid input: " + input) {}
};

class OverflowException : public BigIntException {
public:
    OverflowException(const std::string& operation)
        : BigIntException("Overflow occurred during " + operation) {}
};

// Forward declaration for BigHexInt
class BigHexInt;

// Helper functions for hexadecimal conversions
int convertHexDigitToInt(char c);
char convertIntToHexChar(int n);

// Global map for Karatsuba memoization
// Stores results of sub-problems to avoid redundant calculations
std::map<std::pair<std::string, std::string>, std::string> karatsubaMemo;

// DECIMAL IMPLEMENTATION (BigInt)
class BigInt {
public:
    char digits[MAX_DIGITS]; // Stores digits in reverse order (least significant first)
    int length;
    bool isNegative;

    BigInt() : length(1), isNegative(false) { // Default to 0
        std::fill(digits, digits + MAX_DIGITS, 0);
        digits[0] = 0; // Represents '0'
    }

    BigInt(const std::string& str) {
        *this = createFromString(str);
    }

    static BigInt createFromString(const std::string& str);
    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    int compare(const BigInt& other) const; // -1 if less, 0 if equal, 1 if greater
    void print() const;
    static bool isValidInput(const std::string& str);
    bool isZero() const;
};

// HEXADECIMAL IMPLEMENTATION (BigHexInt)
class BigHexInt {
public:
    char digits[HEX_SIZE]; // Stores hex digits in reverse order (least significant first)
    int length; // Actual number of significant hex digits
    bool isNegative;

    BigHexInt() : length(1), isNegative(false) { // Default to 0x0
        std::fill(digits, digits + HEX_SIZE, '0');
        digits[0] = '0'; // Represents '0'
    }

    // Copy constructor (implicitly generated if not defined, but good to be explicit if needed)
    // BigHexInt(const BigHexInt& other) = default; 

    // Explicitly define copy assignment operator to prevent potential linker issues
    BigHexInt& operator=(const BigHexInt& other) {
        if (this == &other) { // Handle self-assignment
            return *this;
        }
        std::fill(digits, digits + HEX_SIZE, '0'); // Clear existing digits
        for (int i = 0; i < other.length; ++i) {
            digits[i] = other.digits[i];
        }
        length = other.length;
        isNegative = other.isNegative;
        return *this;
    }

    BigHexInt(const std::string& str) {
        *this = createFromString(str);
    }

    static BigHexInt createFromString(const std::string& str);
    BigHexInt operator+(const BigHexInt& other) const;
    BigHexInt operator-(const BigHexInt& other) const;
    BigHexInt operator*(const BigHexInt& other) const;
    BigHexInt operator/(const BigHexInt& other) const;
    BigHexInt operator%(const BigHexInt& other) const;
    
    int compare(const BigHexInt& other) const; // -1 if less, 0 if equal, 1 if greater
    void print() const;
    std::string toString() const;
    static bool isValidInput(const std::string& str);
    
    // Helper methods
    BigHexInt clone() const;
    BigHexInt shiftLeft(int n) const; // Returns new BigHexInt shifted
    void shiftLeftInPlace(int n); // Shifts current BigHexInt
    BigHexInt getLower(int n) const; // Gets lower n digits
    BigHexInt getHigher(int n) const; // Gets higher digits after n
    BigHexInt pad(int targetLen) const; // Pads with leading zeros
    bool isZero() const;
    bool isOne() const;
    bool isEven() const; // Checks if the number is even (LSB is 0)
    BigHexInt addOne() const; // Adds 1 to the number
    BigHexInt subtractOne() const; // Subtracts 1 from the number

    // Primality testing helpers
    BigHexInt modPower(const BigHexInt& exponent, const BigHexInt& modulus) const;
    bool millerRabinTest(int k_iterations) const;

    // Random number generation
    static BigHexInt generateRandom(int numHexDigits);

private:
    BigHexInt multiplyNaive(const BigHexInt& other) const;
    BigHexInt karatsuba(const BigHexInt& other) const;
    // Private declaration for the division helper function
    BigHexInt divide(const BigHexInt& divisor, BigHexInt* remainder = nullptr) const;
};


// Global random device and generator for prime generation
std::random_device rd;
std::mt19937_64 gen(rd()); // Mersenne Twister 64-bit

// Helper function to generate a random hex digit
char getRandomHexDigit() {
    std::uniform_int_distribution<> distrib(0, 15);
    return convertIntToHexChar(distrib(gen));
}

// Helper function to generate a random number within a range (for Miller-Rabin bases)
BigHexInt generateRandomBigHexIntInRange(const BigHexInt& min, const BigHexInt& max) {
    // This is a simplified random generation. For truly large numbers,
    // you'd need a more sophisticated method to generate a uniform random number
    // within the BigHexInt range.
    // For Miller-Rabin, we typically need a random 'a' in [2, n-2].
    // Given the current HEX_SIZE, this approximation is often sufficient.
    
    // Calculate the difference (range size)
    BigHexInt range = max - min;
    if (range.isNegative) { // Should not happen if max >= min
        // BigHexInt temp = min;
        // min = max;
        // max = temp;
        // range = max - min;
        BigHexInt temp(min.toString()); 
        BigHexInt Min(max.toString()); 
        BigHexInt Max(temp.toString());

        range = Max-Min;
    }

    // Determine the number of hex digits needed for the range
    int numDigits = range.length;
    if (numDigits == 1 && range.digits[0] == '0') { // Range is 0, so min == max
        return min;
    }

    BigHexInt random_val;
    do {
        random_val = BigHexInt::generateRandom(numDigits);
        // Ensure random_val is within the range [0, range]
        // This simple method might bias towards smaller numbers if range is not a power of 16
        // A more robust method would involve rejection sampling or bit-by-bit generation.
    } while (random_val.compare(range) > 0);

    return min + random_val;
}


// --- DECIMAL IMPLEMENTATION ---

BigInt BigInt::createFromString(const std::string& str) {
    if (!isValidInput(str)) {
        throw InvalidInputException(str);
    }
    
    BigInt result;
    result.isNegative = false;
    result.length = 0;

    int start = 0;
    if (str[0] == '-') {
        result.isNegative = true;
        start = 1;
    }

    // Remove leading zeros from input string (e.g., "00123" -> "123")
    int actual_start = start;
    while (actual_start < str.length() -1 && str[actual_start] == '0') {
        actual_start++;
    }

    result.length = str.length() - actual_start;
    
    if (result.length > MAX_DIGITS) {
        throw OverflowException("BigInt creation: input too long");
    }

    // Handle "0" specifically
    if (result.length == 1 && str[actual_start] == '0') {
        result.length = 1;
        result.digits[0] = 0;
        result.isNegative = false;
        return result;
    }

    for (int i = 0; i < result.length; i++) {
        result.digits[i] = str[actual_start + result.length - 1 - i] - '0';
    }

    return result;
}

void BigInt::print() const {
    if (isZero()) {
        std::cout << "0\n";
        return;
    }
    if (isNegative) {
        std::cout << "-";
    }
    for (int i = length - 1; i >= 0; i--) {
        std::cout << static_cast<int>(digits[i]);
    }
    std::cout << "\n";
}

int BigInt::compare(const BigInt& other) const {
    if (isNegative && !other.isNegative) return -1;
    if (!isNegative && other.isNegative) return 1;

    // If both are negative, comparison logic is reversed
    int sign_multiplier = isNegative ? -1 : 1;

    if (length != other.length) {
        return (length > other.length ? 1 : -1) * sign_multiplier;
    }

    for (int i = length - 1; i >= 0; i--) {
        if (digits[i] != other.digits[i]) {
            return (digits[i] > other.digits[i] ? 1 : -1) * sign_multiplier;
        }
    }
    return 0; // Equal
}

BigInt BigInt::operator+(const BigInt& other) const {
    if (isNegative == other.isNegative) {
        BigInt result;
        int carry = 0;
        int max_len = std::max(length, other.length);
        result.isNegative = isNegative;
        
        for (int i = 0; i < max_len || carry; i++) {
            int sum = (i < length ? digits[i] : 0) +
                      (i < other.length ? other.digits[i] : 0) + carry;
            
            if (i >= MAX_DIGITS) { // Check for overflow before assigning
                throw OverflowException("addition");
            }
            result.digits[i] = sum % 10;
            carry = sum / 10;
            result.length = i + 1; // Update length as we go
        }
        // If there's a final carry, length is already incremented
        // If the result is 0 (e.g., -5 + 5), ensure length is 1 and digit is 0
        if (result.length == 0) result.length = 1; // Should not happen with current logic, but good for safety
        if (result.digits[result.length - 1] == 0 && result.length > 1) {
            while (result.length > 1 && result.digits[result.length - 1] == 0) {
                result.length--;
            }
        }
        return result;
    } else {
        // Different signs, convert to subtraction
        BigInt absA = *this;
        absA.isNegative = false;
        BigInt absB = other;
        absB.isNegative = false;

        if (absA.compare(absB) >= 0) {
            // |A| - |B| and keep sign of A
            BigInt res = absA - absB;
            res.isNegative = isNegative;
            return res;
        } else {
            // |B| - |A| and keep sign of B
            BigInt res = absB - absA;
            res.isNegative = other.isNegative;
            return res;
        }
    }
}

BigInt BigInt::operator-(const BigInt& other) const {
    if (isNegative != other.isNegative) {
        // Different signs, convert to addition
        BigInt absB = other;
        absB.isNegative = !other.isNegative; // Make it same sign as *this
        return *this + absB;
    }

    BigInt result;
    int borrow = 0;

    // Determine which number is larger in magnitude for subtraction
    int cmp_abs = 0; // 0: equal, 1: *this > other, -1: *this < other
    if (length != other.length) {
        cmp_abs = (length > other.length) ? 1 : -1;
    } else {
        for (int i = length - 1; i >= 0; i--) {
            if (digits[i] != other.digits[i]) {
                cmp_abs = (digits[i] > other.digits[i]) ? 1 : -1;
                break;
            }
        }
    }

    const BigInt *larger_abs, *smaller_abs;
    if (cmp_abs >= 0) { // |*this| >= |other|
        larger_abs = this;
        smaller_abs = &other;
        result.isNegative = isNegative; // Keep original sign
    } else { // |*this| < |other|
        larger_abs = &other;
        smaller_abs = this;
        result.isNegative = !isNegative; // Invert sign
    }

    result.length = larger_abs->length;

    for (int i = 0; i < result.length; i++) {
        int diff = larger_abs->digits[i] - (i < smaller_abs->length ? smaller_abs->digits[i] : 0) - borrow;
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.digits[i] = diff;
    }

    while (result.length > 1 && result.digits[result.length - 1] == 0) {
        result.length--;
    }
    if (result.length == 1 && result.digits[0] == 0) { // Result is zero
        result.isNegative = false;
    }

    return result;
}

BigInt BigInt::operator*(const BigInt& other) const {
    BigInt result;
    std::fill(result.digits, result.digits + MAX_DIGITS, 0);
    
    if (isZero() || other.isZero()) {
        return BigInt("0"); // Result is 0
    }

    result.length = length + other.length;
    if (result.length > MAX_DIGITS) {
        throw OverflowException("multiplication");
    }

    result.isNegative = isNegative != other.isNegative;

    for (int i = 0; i < length; i++) {
        int carry = 0;
        for (int j = 0; j < other.length || carry; j++) {
            int prod = result.digits[i + j] + 
                       digits[i] * (j < other.length ? other.digits[j] : 0) + carry;
            result.digits[i + j] = prod % 10;
            carry = prod / 10;
        }
    }

    while (result.length > 1 && result.digits[result.length - 1] == 0) {
        result.length--;
    }
    return result;
}

bool BigInt::isValidInput(const std::string& str) {
    if (str.empty()) return false;
    if (str.length() > MAX_DIGITS + 1) return false; // +1 for potential minus sign

    int start = 0;
    if (str[0] == '-') {
        start = 1;
        if (str.length() == 1) return false; // Just "-" is invalid
    }
    
    for (size_t i = start; i < str.length(); i++) {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}

bool BigInt::isZero() const {
    return length == 1 && digits[0] == 0;
}


// --- HEXADECIMAL IMPLEMENTATION ---

int convertHexDigitToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // Invalid hex digit
}

char convertIntToHexChar(int n) {
    if (n >= 0 && n <= 9) return '0' + n;
    if (n >= 10 && n < 16) return 'a' + (n - 10);
    throw InvalidInputException("Invalid integer value for hex conversion: " + std::to_string(n));
}

BigHexInt BigHexInt::createFromString(const std::string& str) {
    if (!isValidInput(str)) {
        throw InvalidInputException(str);
    }
    
    BigHexInt result;
    result.isNegative = false;
    std::fill(result.digits, result.digits + HEX_SIZE, '0');
    
    int start = 0;
    if (str[0] == '-') {
        result.isNegative = true;
        start = 1;
    }
    
    // Remove leading zeros from input string (e.g., "00abc" -> "abc")
    int actual_start = start;
    while (actual_start < str.length() -1 && str[actual_start] == '0') {
        actual_start++;
    }

    int inputLength = str.length() - actual_start;
    if (inputLength > HEX_SIZE) {
        throw OverflowException("BigHexInt creation: input too long (max " + std::to_string(HEX_SIZE) + " hex digits)");
    }
    
    // Handle "0" specifically
    if (inputLength == 1 && str[actual_start] == '0') {
        result.length = 1;
        result.digits[0] = '0';
        result.isNegative = false;
        return result;
    }

    for (int i = 0; i < inputLength; i++) {
        result.digits[i] = str[actual_start + inputLength - 1 - i]; // Store in reverse order
    }
    
    result.length = inputLength; // Set length to actual significant digits
    return result;
}

std::string BigHexInt::toString() const {
    std::string result_str;
    if (isNegative && !isZero()) {
        result_str += "-";
    }
    
    // Find the most significant digit
    int msb_idx = length - 1;
    while (msb_idx > 0 && digits[msb_idx] == '0') {
        msb_idx--;
    }
    
    for (int i = msb_idx; i >= 0; i--) {
        result_str += digits[i];
    }
    
    return result_str;
}

void BigHexInt::print() const {
    std::cout << toString() << "\n";
}

int BigHexInt::compare(const BigHexInt& other) const {
    if (isNegative && !other.isNegative) return -1;
    if (!isNegative && other.isNegative) return 1;

    // If both are negative, comparison logic is reversed
    int sign_multiplier = isNegative ? -1 : 1;

    // Compare based on actual lengths first
    if (length != other.length) {
        return (length > other.length ? 1 : -1) * sign_multiplier;
    }

    // Compare digit by digit from most significant
    for (int i = length - 1; i >= 0; i--) {
        int digitA = convertHexDigitToInt(digits[i]);
        int digitB = convertHexDigitToInt(other.digits[i]);
        if (digitA != digitB) {
            return (digitA > digitB ? 1 : -1) * sign_multiplier;
        }
    }
    return 0; // Equal
}

BigHexInt BigHexInt::operator+(const BigHexInt& other) const {
    if (isNegative != other.isNegative) {
        // Different signs, convert to subtraction
        BigHexInt absA = *this;
        absA.isNegative = false;
        BigHexInt absB = other;
        absB.isNegative = false;

        if (absA.compare(absB) >= 0) {
            // |A| - |B| and keep sign of A
            BigHexInt res = absA - absB;
            res.isNegative = isNegative;
            return res;
        } else {
            // |B| - |A| and keep sign of B
            BigHexInt res = absB - absA;
            res.isNegative = other.isNegative;
            return res;
        }
    }
    
    BigHexInt result;
    std::fill(result.digits, result.digits + HEX_SIZE, '0');
    int carry = 0;
    int max_len = std::max(length, other.length);
    result.isNegative = isNegative;
    
    for (int i = 0; i < max_len || carry; i++) {
        int digitA = (i < length) ? convertHexDigitToInt(digits[i]) : 0;
        int digitB = (i < other.length) ? convertHexDigitToInt(other.digits[i]) : 0;
        int sum = digitA + digitB + carry;
        
        if (i >= HEX_SIZE) { // Check for overflow before assigning
            throw OverflowException("hexadecimal addition");
        }
        result.digits[i] = convertIntToHexChar(sum % 16);
        carry = sum / 16;
        result.length = i + 1; // Update length as we go
    }

    // Adjust length if there's a final carry or leading zeros
    if (result.length > HEX_SIZE) { // Check for overflow if carry pushed length beyond HEX_SIZE
        throw OverflowException("hexadecimal addition: result too large");
    }
    while (result.length > 1 && result.digits[result.length - 1] == '0') {
        result.length--;
    }
    return result;
}

BigHexInt BigHexInt::operator-(const BigHexInt& other) const {
    if (isNegative != other.isNegative) {
        // Different signs, convert to addition
        BigHexInt absB = other;
        absB.isNegative = !other.isNegative; // Make it same sign as *this
        return *this + absB;
    }
    
    BigHexInt result;
    std::fill(result.digits, result.digits + HEX_SIZE, '0');
    int borrow = 0;
    
    // Determine which number is larger in magnitude for subtraction
    int cmp_abs = 0; // 0: equal, 1: *this > other, -1: *this < other
    if (length != other.length) {
        cmp_abs = (length > other.length) ? 1 : -1;
    } else {
        for (int i = length - 1; i >= 0; i--) {
            int digitA = convertHexDigitToInt(digits[i]);
            int digitB = convertHexDigitToInt(other.digits[i]);
            if (digitA != digitB) {
                cmp_abs = (digitA > digitB) ? 1 : -1;
                break;
            }
        }
    }
    
    const BigHexInt *larger_abs, *smaller_abs;
    if (cmp_abs >= 0) { // |*this| >= |other|
        larger_abs = this;
        smaller_abs = &other;
        result.isNegative = isNegative; // Keep original sign
    } else { // |*this| < |other|
        larger_abs = &other;
        smaller_abs = this;
        result.isNegative = !isNegative; // Invert sign
    }
    
    result.length = larger_abs->length;
    
    for (int i = 0; i < result.length; i++) {
        int digitL = convertHexDigitToInt(larger_abs->digits[i]);
        int digitS = (i < smaller_abs->length) ? convertHexDigitToInt(smaller_abs->digits[i]) : 0;
        int diff = digitL - digitS - borrow;
        
        if (diff < 0) {
            diff += 16;
            borrow = 1;
        } else {
            borrow = 0;
        }
        
        result.digits[i] = convertIntToHexChar(diff);
    }
    
    while (result.length > 1 && result.digits[result.length - 1] == '0') {
        result.length--;
    }
    if (result.length == 1 && result.digits[0] == '0') { // Result is zero
        result.isNegative = false;
    }
    
    return result;
}

BigHexInt BigHexInt::clone() const {
    BigHexInt result;
    std::fill(result.digits, result.digits + HEX_SIZE, '0');
    for (int i = 0; i < length; i++) {
        result.digits[i] = digits[i];
    }
    result.length = length;
    result.isNegative = isNegative;
    return result;
}

void BigHexInt::shiftLeftInPlace(int n) {
    if (n < 0) {
        throw InvalidInputException("Negative shift amount not supported for shiftLeftInPlace.");
    }
    if (n == 0) return;

    if (isZero()) { // Shifting zero results in zero
        return;
    }

    if (length + n > HEX_SIZE) {
        throw OverflowException("shift left operation: result exceeds max hex digits");
    }
    
    for (int i = length - 1; i >= 0; i--) {
        digits[i + n] = digits[i];
    }

    for (int i = 0; i < n; i++) {
        digits[i] = '0';
    }

    length += n;
}

BigHexInt BigHexInt::shiftLeft(int n) const {
    BigHexInt result = clone();
    result.shiftLeftInPlace(n);
    return result;
}

BigHexInt BigHexInt::getLower(int n) const {
    BigHexInt res;
    std::fill(res.digits, res.digits + HEX_SIZE, '0');
    int actual_len = std::min(length, n);
    for (int i = 0; i < actual_len; i++) {
        res.digits[i] = digits[i];
    }
    res.length = (actual_len == 0) ? 1 : actual_len; // If actual_len is 0, it's "0"
    res.isNegative = false; // Lower part is always positive
    return res;
}

BigHexInt BigHexInt::getHigher(int n) const {
    BigHexInt res;
    std::fill(res.digits, res.digits + HEX_SIZE, '0');
    if (length <= n) { // No higher part
        res.length = 1;
        res.digits[0] = '0';
        res.isNegative = false;
        return res;
    }
    int newLength = length - n;
    for (int i = 0; i < newLength; i++) {
        res.digits[i] = digits[i+n];
    }
    res.length = newLength;
    res.isNegative = false; // Higher part is always positive
    return res;
}

BigHexInt BigHexInt::pad(int targetLen) const {
    BigHexInt res = clone();
    if (res.length < targetLen) {
        if (targetLen > HEX_SIZE) {
            throw OverflowException("padding: target length exceeds max hex digits");
        }
        for (int i = res.length; i < targetLen; i++) {
            res.digits[i] = '0';
        }
        res.length = targetLen;
    }
    return res;
}

BigHexInt BigHexInt::multiplyNaive(const BigHexInt& other) const {
    BigHexInt result;
    std::fill(result.digits, result.digits + HEX_SIZE, '0');
    
    if (isZero() || other.isZero()) {
        return BigHexInt("0");
    }

    // Max possible length is sum of lengths
    if (length + other.length > HEX_SIZE) {
        throw OverflowException("naive multiplication: result too large");
    }

    result.isNegative = isNegative != other.isNegative;

    for (int i = 0; i < length; i++) {
        int carry = 0;
        int a_digit = convertHexDigitToInt(digits[i]);
        for (int j = 0; j < other.length || carry; j++) {
            int b_digit = (j < other.length) ? convertHexDigitToInt(other.digits[j]) : 0;
            int current = convertHexDigitToInt(result.digits[i+j]);
            int prod = current + a_digit * b_digit + carry;
            result.digits[i+j] = convertIntToHexChar(prod % 16);
            carry = prod / 16;
        }
    }

    // Calculate actual length
    int actualLength = length + other.length;
    while (actualLength > 1 && result.digits[actualLength-1] == '0') {
        actualLength--;
    }
    result.length = actualLength;
    return result;
}

BigHexInt BigHexInt::karatsuba(const BigHexInt& other) const {
    // Convert to string for map key. Ensure canonical representation for key.
    std::string thisStr = this->toString();
    std::string otherStr = other.toString();

    // Sort strings to ensure unique key for commutative operation
    if (thisStr > otherStr) {
        std::swap(thisStr, otherStr);
    }
    
    std::pair<std::string, std::string> key = {thisStr, otherStr};

    // Check memoization table
    auto it = karatsubaMemo.find(key);
    if (it != karatsubaMemo.end()) {
        return BigHexInt::createFromString(it->second);
    }

    BigHexInt result;
    
    // Base case for Karatsuba
    if (length <= KARATSUBA_THRESHOLD || other.length <= KARATSUBA_THRESHOLD) {
        result = multiplyNaive(other);
        karatsubaMemo[key] = result.toString(); // Store result
        return result;
    }

    // Handle zero cases
    if (isZero() || other.isZero()) {
        BigHexInt zero_val("0");
        karatsubaMemo[key] = zero_val.toString(); // Store result
        return zero_val;
    }

    int n = std::max(length, other.length);
    // Ensure n is even for splitting, round up if odd
    if (n % 2 != 0) {
        n++;
    }

    // Pad numbers to length n
    BigHexInt x = pad(n);
    BigHexInt y = other.pad(n);

    int m = n / 2; // Split point

    BigHexInt low1 = x.getLower(m);
    BigHexInt high1 = x.getHigher(m);
    BigHexInt low2 = y.getLower(m);
    BigHexInt high2 = y.getHigher(m);

    // Recursive calls
    BigHexInt z0 = low1.karatsuba(low2);
    BigHexInt z2 = high1.karatsuba(high2);

    BigHexInt sum1 = low1 + high1;
    BigHexInt sum2 = low2 + high2;
    BigHexInt z1 = sum1.karatsuba(sum2);

    z1 = z1 - z2;
    z1 = z1 - z0;

    // Combine results
    BigHexInt part1 = z2.shiftLeft(2 * m);
    BigHexInt part2 = z1.shiftLeft(m);
    result = part1 + part2 + z0;

    result.isNegative = isNegative != other.isNegative; // Set final sign
    
    // Adjust length
    while (result.length > 1 && result.digits[result.length-1] == '0') {
        result.length--;
    }

    karatsubaMemo[key] = result.toString(); // Store result
    return result;
}

BigHexInt BigHexInt::operator*(const BigHexInt& other) const {
    // Decide between Karatsuba and Naive based on size
    // The threshold (KARATSUBA_THRESHOLD) can be tuned for performance
    if (length + other.length > KARATSUBA_THRESHOLD * 2) { // Heuristic: if combined length is significant
        return karatsuba(other);
    } else {
        return multiplyNaive(other);
    }
}

// Long division (schoolbook method) for BigHexInt
BigHexInt BigHexInt::operator/(const BigHexInt& divisor) const {
    BigHexInt remainder_val;
    BigHexInt quotient = divide(divisor, &remainder_val);
    return quotient;
}

BigHexInt BigHexInt::operator%(const BigHexInt& divisor) const {
    BigHexInt remainder_val;
    divide(divisor, &remainder_val);
    return remainder_val;
}

// Private helper for division, used by both / and %
BigHexInt BigHexInt::divide(const BigHexInt& divisor_abs, BigHexInt* remainder_ptr) const {
    BigHexInt abs_this = *this;
    abs_this.isNegative = false; // Work with absolute values
    BigHexInt abs_divisor = divisor_abs;
    abs_divisor.isNegative = false;

    if (abs_divisor.isZero()) {
        throw DivisionByZeroException();
    }
    if (abs_this.isZero()) {
        if (remainder_ptr != nullptr) *remainder_ptr = BigHexInt("0");
        return BigHexInt("0");
    }

    // If dividend is smaller than divisor, quotient is 0, remainder is dividend
    if (abs_this.compare(abs_divisor) < 0) {
        if (remainder_ptr != nullptr) *remainder_ptr = abs_this;
        return BigHexInt("0");
    }

    BigHexInt quotient;
    std::fill(quotient.digits, quotient.digits + HEX_SIZE, '0');
    quotient.length = 1; // Start with length 1, will adjust later

    BigHexInt current_dividend_part;
    std::fill(current_dividend_part.digits, current_dividend_part.digits + HEX_SIZE, '0');
    current_dividend_part.length = 1;

    // The current_dividend_part will be built by taking digits from the dividend
    // from MSB to LSB.
    // We need to iterate from the most significant digit of the dividend
    // to the least significant.
    
    // Initialize current_dividend_part with the first 'divisor.length' digits of 'this'
    // or more if 'this' is shorter than 'divisor'
    int dividend_msb_idx = length - 1;
    int divisor_len = abs_divisor.length;

    // Start building the current_dividend_part from the most significant digits
    // The loop iterates from the most significant digit of the dividend down to 0
    // to simulate long division.
    for (int i = dividend_msb_idx; i >= 0; --i) {
        // Shift current_dividend_part left by one hex digit
        // This is equivalent to multiplying by 16
        for (int k = current_dividend_part.length; k > 0; --k) {
            current_dividend_part.digits[k] = current_dividend_part.digits[k-1];
        }
        current_dividend_part.digits[0] = digits[i]; // Add the new digit
        current_dividend_part.length++;
        while (current_dividend_part.length > 1 && current_dividend_part.digits[current_dividend_part.length - 1] == '0') {
            current_dividend_part.length--;
        }

        // If current_dividend_part is still smaller than divisor, the quotient digit is 0
        // and we continue to the next digit of the dividend.
        if (current_dividend_part.compare(abs_divisor) < 0) {
            // If this is the first part and it's too small, the quotient digit is 0.
            // Only append '0' to quotient if it's not the very first digit and not already '0'.
            if (quotient.length > 1 || quotient.digits[0] != '0') {
                quotient.shiftLeftInPlace(1);
            }
            continue;
        }

        // Estimate the quotient digit
        int q_digit = 0;
        // Simple linear search for quotient digit (can be optimized with binary search)
        BigHexInt temp_divisor_mult = abs_divisor.clone();
        for (int d = 1; d < 16; ++d) { // Try digits 1 to F
            BigHexInt next_mult = abs_divisor.multiplyNaive(BigHexInt(std::string(1, convertIntToHexChar(d))));
            if (current_dividend_part.compare(next_mult) >= 0) {
                q_digit = d;
            } else {
                break;
            }
        }
        
        // Subtract (q_digit * divisor) from current_dividend_part
        BigHexInt subtract_val = abs_divisor.multiplyNaive(BigHexInt(std::string(1, convertIntToHexChar(q_digit))));
        current_dividend_part = current_dividend_part - subtract_val;

        // Append q_digit to the quotient
        // We are building the quotient from MSB to LSB, so we need to shift existing digits
        // and add the new one.
        for (int k = quotient.length; k > 0; --k) {
            quotient.digits[k] = quotient.digits[k-1];
        }
        quotient.digits[0] = convertIntToHexChar(q_digit);
        quotient.length++;
        while (quotient.length > 1 && quotient.digits[quotient.length - 1] == '0') {
            quotient.length--;
        }
    }

    // Set quotient sign
    quotient.isNegative = (this->isNegative != divisor_abs.isNegative);
    if (quotient.isZero()) quotient.isNegative = false;

    // Set remainder sign (remainder has same sign as dividend)
    if (remainder_ptr != nullptr) {
        *remainder_ptr = current_dividend_part;
        remainder_ptr->isNegative = this->isNegative;
        if (remainder_ptr->isZero()) remainder_ptr->isNegative = false;
    }

    return quotient;
}


bool BigHexInt::isZero() const {
    return length == 1 && digits[0] == '0';
}

bool BigHexInt::isOne() const {
    return length == 1 && digits[0] == '1' && !isNegative;
}

bool BigHexInt::isEven() const {
    // A hexadecimal number is even if its last digit is 0, 2, 4, 6, 8, A, C, E
    // In terms of integer values, if the LSB (digits[0]) is even.
    return (convertHexDigitToInt(digits[0]) % 2 == 0);
}

BigHexInt BigHexInt::addOne() const {
    return *this + BigHexInt("1");
}

BigHexInt BigHexInt::subtractOne() const {
    return *this - BigHexInt("1");
}

bool BigHexInt::isValidInput(const std::string& str) {
    if (str.empty()) return false;
    if (str.length() > HEX_SIZE + 1) return false; // +1 for potential minus sign
    
    int start = 0;
    if (str[0] == '-') {
        start = 1;
        if (str.length() == 1) return false; // Just "-" is invalid
    }

    for (size_t i = start; i < str.length(); i++) {
        char c = str[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    return true;
}

// Modular Exponentiation: (base^exponent) % modulus
BigHexInt BigHexInt::modPower(const BigHexInt& exponent, const BigHexInt& modulus) const {
    BigHexInt res("1"); // Initialize result to 1
    BigHexInt base = *this;
    base = base % modulus; // base = base % modulus

    BigHexInt exp = exponent;
    BigHexInt zero("0");
    BigHexInt one("1");
    BigHexInt two("2");

    while (exp.compare(zero) > 0) {
        // If exponent is odd
        if (convertHexDigitToInt(exp.digits[0]) % 2 != 0) { // Check LSB for oddness
            res = (res * base) % modulus;
        }
        // exponent = exponent / 2
        exp = exp / two;
        // base = (base * base) % modulus
        base = (base * base) % modulus;
    }
    return res;
}

// Miller-Rabin Primality Test
// k_iterations: number of bases to test against (higher k means more certainty)
bool BigHexInt::millerRabinTest(int k_iterations) const {
    BigHexInt n = *this;
    BigHexInt zero("0");
    BigHexInt one("1");
    BigHexInt two("2");
    BigHexInt three("3");

    // Handle small numbers and even numbers
    if (n.compare(one) <= 0) return false; // n <= 1 is not prime
    if (n.compare(two) == 0 || n.compare(three) == 0) return true; // 2 and 3 are prime
    if (n.isEven()) return false; // Even numbers > 2 are not prime

    // Write n-1 as d * 2^s
    BigHexInt n_minus_1 = n.subtractOne();
    BigHexInt d = n_minus_1;
    int s = 0;

    // While d is even, divide by 2 and increment s
    while (d.isEven()) {
        d = d / two;
        s++;
    }

    // Perform k_iterations of the test
    for (int i = 0; i < k_iterations; ++i) {
        // Choose a random base 'a' such that 2 <= a <= n-2
        BigHexInt a = generateRandomBigHexIntInRange(two, n.subtractOne().subtractOne()); // n-2

        BigHexInt x = a.modPower(d, n); // x = a^d % n

        if (x.isOne() || x.compare(n_minus_1) == 0) {
            continue; // Probably prime, try next base
        }

        bool composite = true;
        for (int r = 0; r < s; ++r) {
            x = (x * x) % n;
            if (x.isOne()) { // x = 1 implies composite (unless it was 1 or n-1 initially)
                composite = true; // Still composite, but this path is not the strong one
                break;
            }
            if (x.compare(n_minus_1) == 0) {
                composite = false; // Found a strong probable prime
                break;
            }
        }

        if (composite) {
            return false; // Definitely composite
        }
    }

    return true; // Probably prime
}

// Generate a random BigHexInt with specified number of hex digits
BigHexInt BigHexInt::generateRandom(int numHexDigits) {
    if (numHexDigits <= 0 || numHexDigits > HEX_SIZE) {
        throw InvalidInputException("Invalid number of hex digits for random generation.");
    }

    BigHexInt result;
    std::fill(result.digits, result.digits + HEX_SIZE, '0');
    result.isNegative = false;
    result.length = numHexDigits;

    // Ensure the most significant digit is not '0'
    result.digits[numHexDigits - 1] = getRandomHexDigit();
    while (result.digits[numHexDigits - 1] == '0') {
        result.digits[numHexDigits - 1] = getRandomHexDigit();
    }

    // Fill remaining digits randomly
    for (int i = 0; i < numHexDigits - 1; ++i) {
        result.digits[i] = getRandomHexDigit();
    }

    // Ensure the number is odd (set LSB to 1)
    result.digits[0] = convertIntToHexChar(convertHexDigitToInt(result.digits[0]) | 1);

    // Update length just in case (though it should be numHexDigits)
    while (result.length > 1 && result.digits[result.length - 1] == '0') {
        result.length--;
    }
    return result;
}


// Function to generate a prime number
BigHexInt generatePrime(int numHexDigits, int millerRabinIterations) {
    std::cout << "Generating a " << numHexDigits << "-hexabit prime...\n";
    BigHexInt candidate;
    BigHexInt two("2");
    BigHexInt three("3");
    BigHexInt five("5");
    BigHexInt seven("7");
    BigHexInt eleven("11");
    BigHexInt thirteen("13");
    BigHexInt seventeen("17");
    BigHexInt nineteen("19");

    while (true) {
        candidate = BigHexInt::generateRandom(numHexDigits);
        
        // Small prime sieve: check divisibility by small primes
        // This quickly eliminates many composites before the expensive Miller-Rabin test
        if (candidate.isZero() || candidate.isOne()) continue;
        if (candidate.compare(two) == 0 || candidate.compare(three) == 0 ||
            candidate.compare(five) == 0 || candidate.compare(seven) == 0 ||
            candidate.compare(eleven) == 0 || candidate.compare(thirteen) == 0) {
            // If the candidate itself is one of these small primes, it's prime.
            // But we're generating large random numbers, so this is unlikely.
            // If it's equal to a small prime, it's prime, but we'll still run Miller-Rabin for consistency.
        } else if (candidate.isEven() || // Already handled by generateRandom setting LSB to 1, but good for safety
                   (candidate % three).isZero() ||
                   (candidate % five).isZero() ||
                   (candidate % seven).isZero() ||
                   (candidate % eleven).isZero() ||
                   (candidate % seventeen).isZero() ||
                   (candidate % nineteen).isZero() ||
                   (candidate % thirteen).isZero()) {
            std::cout << "Candidate " << candidate.toString() << " eliminated by small prime sieve. Trying next...\n";
            continue; // Not prime, try next candidate
        }

        // If it passes the small prime sieve, run Miller-Rabin
        std::cout << "Testing candidate: " << candidate.toString() << " with Miller-Rabin...\n";
        if (candidate.millerRabinTest(millerRabinIterations)) {
            std::cout << "Found prime: " << candidate.toString() << "\n";
            return candidate;
        } else {
            std::cout << "Candidate " << candidate.toString() << " failed Miller-Rabin. Trying next...\n";
            // Increment by 2 to keep it odd and try again (step-and-test)
            candidate = candidate.addOne().addOne(); // candidate + 2
        }
    }
}
void runTests()
{
    std::cout<<"No tests available"<<std::endl;
}
 
void runDiffieHellmanSimulation() {
    std::cout << "\n--- Diffie-Hellman Key Exchange Simulation ---\n";

    // Step 1: Automatically generate a large prime number (p) and set base (g)
    std::cout << "Generating a large prime number for the simulation...\n";
    
    // Generate a prime with reasonable size (8 hex digits = 32-bit prime)
    // You can increase this for stronger security (e.g., 16 for 64-bit, 32 for 128-bit)
    int primeHexDigits = 64;
    int mrIterations = 20;
    
    BigHexInt p = generatePrime(primeHexDigits, mrIterations);
    std::cout << "Generated prime (p): " << p.toString() << "\n";

    // Use a common base value (generator)
    // 2, 5, and 7 are commonly used generators
    BigHexInt g("7"); // Using 7 as the base
    std::cout << "Using base (g): " << g.toString() << "\n";

    // Ensure g is less than p (should always be true with our small g values)
    if (g.compare(p) >= 0) {
        std::cout << "Error: Generated prime is too small. Regenerating...\n";
        // Fallback: use a larger prime
        p = generatePrime(primeHexDigits + 2, mrIterations);
        std::cout << "New generated prime (p): " << p.toString() << "\n";
    }

    // Step 2: Alice chooses a private key (a)
    // This should be a large random number, less than p.
    int private_key_hex_digits = p.length / 2; // Roughly half the digits of p for a reasonable size
    if (private_key_hex_digits < 2) private_key_hex_digits = 2; // Minimum 2 hex digits
    
    BigHexInt alice_private_key_a = BigHexInt::generateRandom(private_key_hex_digits);
    // Ensure private key is less than p
    while (alice_private_key_a.compare(p) >= 0) {
        alice_private_key_a = BigHexInt::generateRandom(private_key_hex_digits);
    }
    std::cout << "\nAlice's private key (a): " << alice_private_key_a.toString() << "\n";

    // Step 3: Bob chooses a private key (b)
    BigHexInt bob_private_key_b = BigHexInt::generateRandom(private_key_hex_digits);
    // Ensure private key is less than p
    while (bob_private_key_b.compare(p) >= 0) {
        bob_private_key_b = BigHexInt::generateRandom(private_key_hex_digits);
    }
    std::cout << "Bob's private key (b):   " << bob_private_key_b.toString() << "\n";

    // Step 4: Alice computes her public key (A) = g^a mod p
    std::cout << "\nAlice computing public key A = g^a mod p...\n";
    BigHexInt alice_public_key_A = g.modPower(alice_private_key_a, p);
    std::cout << "Alice's public key (A):  " << alice_public_key_A.toString() << "\n";

    // Step 5: Bob computes his public key (B) = g^b mod p
    std::cout << "Bob computing public key B = g^b mod p...\n";
    BigHexInt bob_public_key_B = g.modPower(bob_private_key_b, p);
    std::cout << "Bob's public key (B):    " << bob_public_key_B.toString() << "\n";

    // Public keys A and B are exchanged over an insecure channel.

    // Step 6: Alice computes the shared secret key (S_A) = B^a mod p
    std::cout << "\nAlice computing shared secret S_A = B^a mod p...\n";
    BigHexInt alice_shared_secret_SA = bob_public_key_B.modPower(alice_private_key_a, p);
    std::cout << "Alice's shared secret (S_A): " << alice_shared_secret_SA.toString() << "\n";

    // Step 7: Bob computes the shared secret key (S_B) = A^b mod p
    std::cout << "Bob computing shared secret S_B = A^b mod p...\n";
    BigHexInt bob_shared_secret_SB = alice_public_key_A.modPower(bob_private_key_b, p);
    std::cout << "Bob's shared secret (S_B):   " << bob_shared_secret_SB.toString() << "\n";

    // Step 8: Verify shared secrets
    std::cout << "\n--- Verification ---\n";
    if (alice_shared_secret_SA.compare(bob_shared_secret_SB) == 0) {
        std::cout << "Shared secrets match! Diffie-Hellman Key Exchange successful.\n";
    } else {
        std::cout << "Error: Shared secrets DO NOT match. Diffie-Hellman Key Exchange FAILED.\n";
    }
    std::cout << "-------------------------------------\n";
}

// Function to convert a string message to hexadecimal representation
std::string stringToHex(const std::string& message) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char c : message) {
        ss << std::setw(2) << static_cast<unsigned>(c);
    }
    return ss.str();
}

// Function to convert hexadecimal string back to original string
std::string hexToString(const std::string& hex) {
    std::string result;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byte = hex.substr(i, 2);
        char c = static_cast<char>(std::strtoul(byte.c_str(), nullptr, 16));
        result += c;
    }
    return result;
}

// Function to pad hex string to make it divisible by chunk size
std::string padHexString(const std::string& hex, size_t chunkSize) {
    std::string padded = hex;
    size_t remainder = hex.length() % chunkSize;
    if (remainder != 0) {
        size_t paddingNeeded = chunkSize - remainder;
        padded += std::string(paddingNeeded, '0');
    }
    return padded;
}

// Function to remove padding from decrypted hex string
std::string removePadding(const std::string& hex) {
    // Remove trailing zeros that were added as padding
    std::string result = hex;
    while (!result.empty() && result.back() == '0') {
        result.pop_back();
    }
    // If we removed too many zeros (in case the original message ended with null chars),
    // we need a more sophisticated padding scheme. For simplicity, we'll use this approach.
    return result;
}

// Function to encrypt/decrypt message using XOR with the shared secret key
std::vector<std::string> encryptDecryptMessage(const std::string& message, const BigHexInt& sharedSecret) {
    std::cout << "\n=== Message Processing ===\n";
    std::cout << "Original message: \"" << message << "\"\n";
    
    // Convert message to hex
    std::string messageHex = stringToHex(message);
    std::cout << "Message in hex: " << messageHex << "\n";
    
    // Get the shared secret as hex string
    std::string secretKey = sharedSecret.toString();
    std::cout << "Shared secret key: " << secretKey << "\n";
    
    // Determine chunk size based on secret key length
    size_t chunkSize = secretKey.length();
    std::cout << "Using chunk size: " << chunkSize << " hex characters\n";
    
    // Pad message hex to be divisible by chunk size
    std::string paddedMessageHex = padHexString(messageHex, chunkSize);
    std::cout << "Padded message hex: " << paddedMessageHex << "\n";
    
    // Split message into chunks and XOR each chunk with the secret key
    std::vector<std::string> encryptedChunks;
    std::cout << "\nProcessing chunks:\n";
    
    for (size_t i = 0; i < paddedMessageHex.length(); i += chunkSize) {
        std::string chunk = paddedMessageHex.substr(i, chunkSize);
        std::cout << "Chunk " << (i/chunkSize + 1) << ": " << chunk;
        
        // XOR chunk with secret key
        BigHexInt chunkBig(chunk);
        BigHexInt secretBig(secretKey);
        
        // Perform XOR operation (we'll need to implement XOR in BigHexInt, 
        // for now let's simulate it with a simple character-wise XOR)
        std::string encryptedChunk = "";
        for (size_t j = 0; j < chunk.length(); ++j) {
            char chunkChar = chunk[j];
            char secretChar = secretKey[j % secretKey.length()];
            
            // Convert hex characters to integers, XOR them, convert back
            int chunkVal = (chunkChar >= '0' && chunkChar <= '9') ? chunkChar - '0' : chunkChar - 'a' + 10;
            int secretVal = (secretChar >= '0' && secretChar <= '9') ? secretChar - '0' : secretChar - 'a' + 10;
            int xorResult = chunkVal ^ secretVal;
            
            char resultChar = (xorResult < 10) ? '0' + xorResult : 'a' + xorResult - 10;
            encryptedChunk += resultChar;
        }
        
        encryptedChunks.push_back(encryptedChunk);
        std::cout << " -> Encrypted: " << encryptedChunk << "\n";
    }
    
    return encryptedChunks;
}

// Function to decrypt the chunks back to original message
std::string decryptMessage(const std::vector<std::string>& encryptedChunks, const BigHexInt& sharedSecret) {
    std::cout << "\n=== Message Decryption ===\n";
    
    std::string secretKey = sharedSecret.toString();
    std::cout << "Using shared secret key: " << secretKey << "\n";
    
    std::string decryptedHex = "";
    std::cout << "Decrypting chunks:\n";
    
    for (size_t i = 0; i < encryptedChunks.size(); ++i) {
        std::string encryptedChunk = encryptedChunks[i];
        std::cout << "Encrypted chunk " << (i + 1) << ": " << encryptedChunk;
        
        // XOR back with secret key (XOR is its own inverse)
        std::string decryptedChunk = "";
        for (size_t j = 0; j < encryptedChunk.length(); ++j) {
            char encChar = encryptedChunk[j];
            char secretChar = secretKey[j % secretKey.length()];
            
            int encVal = (encChar >= '0' && encChar <= '9') ? encChar - '0' : encChar - 'a' + 10;
            int secretVal = (secretChar >= '0' && secretChar <= '9') ? secretChar - '0' : secretChar - 'a' + 10;
            int xorResult = encVal ^ secretVal;
            
            char resultChar = (xorResult < 10) ? '0' + xorResult : 'a' + xorResult - 10;
            decryptedChunk += resultChar;
        }
        
        decryptedHex += decryptedChunk;
        std::cout << " -> Decrypted: " << decryptedChunk << "\n";
    }
    
    std::cout << "Full decrypted hex: " << decryptedHex << "\n";
    
    // Remove padding and convert back to string
    std::string cleanHex = removePadding(decryptedHex);
    std::string originalMessage = hexToString(cleanHex);
    
    std::cout << "Decrypted message: \"" << originalMessage << "\"\n";
    return originalMessage;
}


// Updated DHKE simulation with message encryption/decryption
void runDiffieHellmanWithEncryption() {
    std::cout << "\n--- Diffie-Hellman Key Exchange with Message Encryption ---\n";

    // Step 1: Automatically generate a large prime number (p) and set base (g)
    std::cout << "Generating a large prime number for the simulation...\n";
    
    int primeHexDigits = 64;
    int mrIterations = 25;
    
    BigHexInt p = generatePrime(primeHexDigits, mrIterations);
    std::cout << "Generated prime (p): " << p.toString() << "\n";

    BigHexInt g("7");
    std::cout << "Using base (g): " << g.toString() << "\n";

    if (g.compare(p) >= 0) {
        std::cout << "Error: Generated prime is too small. Regenerating...\n";
        p = generatePrime(primeHexDigits + 2, mrIterations);
        std::cout << "New generated prime (p): " << p.toString() << "\n";
    }

    // Step 2-7: Standard DHKE process
    int private_key_hex_digits = p.length / 2;
    if (private_key_hex_digits < 2) private_key_hex_digits = 2;
    
    BigHexInt alice_private_key_a = BigHexInt::generateRandom(private_key_hex_digits);
    while (alice_private_key_a.compare(p) >= 0) {
        alice_private_key_a = BigHexInt::generateRandom(private_key_hex_digits);
    }
    std::cout << "\nAlice's private key (a): " << alice_private_key_a.toString() << "\n";

    BigHexInt bob_private_key_b = BigHexInt::generateRandom(private_key_hex_digits);
    while (bob_private_key_b.compare(p) >= 0) {
        bob_private_key_b = BigHexInt::generateRandom(private_key_hex_digits);
    }
    std::cout << "Bob's private key (b):   " << bob_private_key_b.toString() << "\n";

    std::cout << "\nAlice computing public key A = g^a mod p...\n";
    BigHexInt alice_public_key_A = g.modPower(alice_private_key_a, p);
    std::cout << "Alice's public key (A):  " << alice_public_key_A.toString() << "\n";

    std::cout << "Bob computing public key B = g^b mod p...\n";
    BigHexInt bob_public_key_B = g.modPower(bob_private_key_b, p);
    std::cout << "Bob's public key (B):    " << bob_public_key_B.toString() << "\n";

    std::cout << "\nAlice computing shared secret S_A = B^a mod p...\n";
    BigHexInt alice_shared_secret_SA = bob_public_key_B.modPower(alice_private_key_a, p);
    std::cout << "Alice's shared secret (S_A): " << alice_shared_secret_SA.toString() << "\n";

    std::cout << "Bob computing shared secret S_B = A^b mod p...\n";
    BigHexInt bob_shared_secret_SB = alice_public_key_A.modPower(bob_private_key_b, p);
    std::cout << "Bob's shared secret (S_B):   " << bob_shared_secret_SB.toString() << "\n";

    // Verify shared secrets match
    std::cout << "\n--- Verification ---\n";
    if (alice_shared_secret_SA.compare(bob_shared_secret_SB) == 0) {
        std::cout << "Shared secrets match! Diffie-Hellman Key Exchange successful.\n";
        
        // Step 8: Message encryption and transmission simulation
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "SECURE MESSAGE TRANSMISSION SIMULATION\n";
        std::cout << std::string(50, '=') << "\n";
        
        // Get message from user
        std::cout << "\nEnter a message for Alice to send to Bob: ";
        std::cin.ignore(); // Clear any remaining newline
        std::string message;
        std::getline(std::cin, message);
        
        std::cout << "\n--- ALICE ENCRYPTS MESSAGE ---\n";
        // Alice encrypts the message
        std::vector<std::string> encryptedChunks = encryptDecryptMessage(message, alice_shared_secret_SA);
        
        std::cout << "\n--- MESSAGE TRANSMISSION (Insecure Channel) ---\n";
        std::cout << "Encrypted chunks being transmitted:\n";
        for (size_t i = 0; i < encryptedChunks.size(); ++i) {
            std::cout << "Chunk " << (i + 1) << ": " << encryptedChunks[i] << "\n";
        }
        
        std::cout << "\n--- BOB RECEIVES AND DECRYPTS MESSAGE ---\n";
        // Bob decrypts the message using his shared secret (which should be identical)
        std::string decryptedMessage = decryptMessage(encryptedChunks, bob_shared_secret_SB);
        
        std::cout << "\n--- FINAL VERIFICATION ---\n";
        if (message == decryptedMessage) {
            std::cout << "SUCCESS! Message was encrypted and decrypted correctly.\n";
            std::cout << "Original:  \"" << message << "\"\n";
            std::cout << "Decrypted: \"" << decryptedMessage << "\"\n";
        } else {
            std::cout << "ERROR! Message corruption detected.\n";
            std::cout << "Original:  \"" << message << "\"\n";
            std::cout << "Decrypted: \"" << decryptedMessage << "\"\n";
        }
        
    } else {
        std::cout << "Error: Shared secrets DO NOT match. Cannot proceed with encryption.\n";
    }
    std::cout << "\n" << std::string(50, '=') << "\n";
}

int main() {
    // Seed the random number generator
    gen.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    try {
        std::cout << "Welcome to the Big Integer Calculator and Prime Generator!\n";
        std::cout << "Enter 'T' for test suite, 'M' for interactive mode, 'D' for basic DHKE, or 'E' for DHKE with encryption: ";
        char mode_choice;
        std::cin >> mode_choice;
        std::cin.ignore(); // Consume the newline

        if (mode_choice == 'T' || mode_choice == 't') {
            // Run the automated test suite
            runTests();
        } else if (mode_choice == 'D' || mode_choice == 'd') {
            // Run basic Diffie-Hellman simulation
            runDiffieHellmanSimulation();
        } else if (mode_choice == 'E' || mode_choice == 'e') {
            // Run Diffie-Hellman with message encryption
            runDiffieHellmanWithEncryption();
        } else if (mode_choice == 'M' || mode_choice == 'm') {
            std::cout << "Entering Interactive Mode.\n";
            std::cout << "Enter 'H' for Hexadecimal operations or 'D' for Decimal operations.\n";
            char op_mode_char;
            std::cin >> op_mode_char;
            bool isHexMode = (op_mode_char == 'H' || op_mode_char == 'h');

            if (isHexMode) {
                std::cout << "Entering Hexadecimal Calculator/Prime Generation Mode.\n";
                std::cout << "Enter number of test cases for calculations, or '0' to generate a prime: ";
                int test_cases;
                std::cin >> test_cases;
                std::cin.ignore(); // Consume the newline

                if (test_cases == 0) {
                    // Prime generation mode
                    int numHexDigits = 8; // Example: 8 hex digits for a 32-bit number
                    int mrIterations = 10; // Number of Miller-Rabin iterations

                    std::cout << "Generating a prime number with " << numHexDigits << " hex digits.\n";
                    std::cout << "Miller-Rabin iterations per test: " << mrIterations << "\n";
                    BigHexInt prime = generatePrime(numHexDigits, mrIterations);
                    std::cout << "\nGenerated Prime: " << prime.toString() << "\n";

                } else {
                    // Hexadecimal calculation mode
                    for (int t = 0; t < test_cases; ++t) {
                        char op;
                        std::string num1_str, num2_str;

                        std::cout << "Enter operation (+, -, *, /, %) and two hexadecimal numbers (e.g., + 123 ABC): ";
                        std::cin >> op >> num1_str >> num2_str;

                        try {
                            BigHexInt a(num1_str), b(num2_str), result;
                            switch (op) {
                                case '+': result = a + b; break;
                                case '-': result = a - b; break;
                                case '*': result = a * b; break;
                                case '/': result = a / b; break;
                                case '%': result = a % b; break;
                                default:
                                    std::cout << "Invalid operator: " << op << "\n";
                                    continue;
                            }
                            std::cout << "Result: ";
                            result.print();
                        }
                        catch (const BigIntException& e) {
                            std::cout << "Error in calculation: " << e.what() << "\n";
                        }
                    }
                }
            } else {
                std::cout << "Entering Decimal Calculator Mode.\n";
                std::cout << "Enter number of test cases: ";
                int test_cases;
                std::cin >> test_cases;
                std::cin.ignore(); // Consume the newline

                for (int t = 0; t < test_cases; ++t) {
                    char op;
                    std::string num1_str, num2_str;

                    std::cout << "Enter operation (+, -, *) and two decimal numbers (e.g., + 123 456): ";
                    std::cin >> op >> num1_str >> num2_str;

                    try {
                        BigInt a(num1_str), b(num2_str), result;
                        switch (op) {
                            case '+': result = a + b; break;
                            case '-': result = a - b; break;
                            case '*': result = a * b; break;
                            case '/':
                            case '%':
                                std::cout << "Division/Modulo only supported for hexadecimal in this implementation.\n";
                                continue;
                            default:
                                std::cout << "Invalid operator: " << op << "\n";
                                continue;
                        }
                        std::cout << "Result: ";
                        result.print();
                    }
                    catch (const BigIntException& e) {
                        std::cout << "Error in calculation: " << e.what() << "\n";
                    }
                }
            }
        } else {
            std::cout << "Invalid choice. Exiting.\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}