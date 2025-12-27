#include "Bigint.hpp"
#include "exceptions.hpp"
#include "Timer.hpp"

//constructors
BigInt::BigInt() : length(0), isNegative(false) {
        std::fill(digits, digits + MAX_DIGITS, 0);
}

BigInt::BigInt(const std::string& str) {
        *this = createFromString(str);
}

BigHexInt::BigHexInt() : length(1), isNegative(false) {
        std::fill(digits, digits + MAX_HEX_RESULT_SIZE, '0');
}

BigHexInt::BigHexInt(const std::string& str) {
        *this = createFromString(str);
}

// Global variable definitions
std::map<std::pair<std::string, std::string>, std::string> karatsubaMemo;
int hexMultiplyLookup[HEX_LOOKUP_SIZE][HEX_LOOKUP_SIZE];



//-------------------- DECIMAL BIGINT IMPLEMENTATION --------------------//

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

    result.length = str.length() - start;
    
    if (result.length > MAX_DIGITS) {
        throw OverflowException("BigInt creation");
    }

    for (int i = 0; i < result.length; i++) {
        result.digits[i] = str[result.length - 1 - i + start] - '0';
    }

    for (int i = result.length; i < MAX_DIGITS; i++) {
        result.digits[i] = 0;
    }

    return result;
}

void BigInt::print() const {
    if (isNegative) {
        std::cout << "-";
    }
    for (int i = length - 1; i >= 0; i--) {
        std::cout << static_cast<int>(digits[i]);
    }
    std::cout << std::endl;
}

int BigInt::compare(const BigInt& other) const {
    if (length != other.length) {
        return (length > other.length) ? 1 : -1;
    }

    for (int i = length - 1; i >= 0; i--) {
        if (digits[i] != other.digits[i]) {
            return (digits[i] > other.digits[i]) ? 1 : -1;
        }
    }
    return 0;
}

BigInt BigInt::operator+(const BigInt& other) const {
    if (isNegative == other.isNegative) {
        BigInt result;
        int carry = 0;
        result.length = std::max(length, other.length);
        result.isNegative = isNegative;
        
        if (result.length >= MAX_DIGITS - 1) {
            throw OverflowException("addition");
        }

        for (int i = 0; i < result.length || carry; i++) {
            int sum = (i < length ? digits[i] : 0) +
                      (i < other.length ? other.digits[i] : 0) + carry;
            result.digits[i] = sum % 10;
            carry = sum / 10;
            if (i == result.length) {                  //<-changed the >= to == need to check if it works 
                result.length++;
            }
        }
        return result;
    } else {
        if (isNegative) {
            BigInt absA = *this;
            absA.isNegative = false;
            return other - absA;
        } else {
            BigInt absB = other;
            absB.isNegative = false;
            return *this - absB;
        }
    }
}

BigInt BigInt::operator-(const BigInt& other) const {
    if (isNegative != other.isNegative) {
        BigInt absB = other;
        absB.isNegative = !other.isNegative;
        return *this + absB;
    }

    BigInt result;
    int borrow = 0;

    if (compare(other) < 0) {
        result = other - *this;
        result.isNegative = !isNegative;
        return result;
    }

    result.length = length;
    result.isNegative = isNegative;

    for (int i = 0; i < result.length; i++) {
        int diff = digits[i] - (i < other.length ? other.digits[i] : 0) - borrow;
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

    return result;
}

BigInt BigInt::operator*(const BigInt& other) const {
    BigInt result;
    std::fill(result.digits, result.digits + MAX_DIGITS, 0);
    result.length = length + other.length;
    result.isNegative = isNegative != other.isNegative;
    
    if (result.length >= MAX_DIGITS) {
        throw OverflowException("multiplication");
    }

    for (int i = 0; i < length; i++) {
        int carry = 0;
        for (int j = 0; j < other.length || carry; j++) {
            int prod = result.digits[i + j] + digits[i] * (j < other.length ? other.digits[j] : 0) + carry;
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
    if (str[0] == '-' && str.length() == 1) return false;
    if (str[0] != '-' && (str[0] < '0' || str[0] > '9')) return false;
    
    for (size_t i = 1; i < str.length(); i++) {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}

//-------------------- HEXADECIMAL BIGINT IMPLEMENTATION --------------------//

int convertHexDigitToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

char convertIntToHexChar(int n) {
    if (n >= 0 && n <= 9) return '0' + n;
    if (n >= 10 && n < 16) return 'a' + (n - 10);
    throw InvalidInputException("Invalid isHex digit value: " + std::to_string(n));
}

BigHexInt BigHexInt::createFromString(const std::string& str) {
    if (!isValidInput(str)) {
        throw InvalidInputException(str);
    }
    
    BigHexInt result;
    result.isNegative = false;
    std::fill(result.digits, result.digits + MAX_HEX_RESULT_SIZE, '0');
    
    int start = 0;
    if (str[0] == '-') {
        result.isNegative = true;
        start = 1;
    }
    
    int inputLength = str.length() - start;
    if (inputLength > HEX_SIZE) {
        throw OverflowException("BigHexInt creation - exceeds " + std::to_string(HEX_SIZE) + " isHex digits");
    }
    
    for (int i = 0; i < inputLength; i++) {
        result.digits[i] = str[start + inputLength - 1 - i];
    }
    
    int actualLength = HEX_SIZE;
    while (actualLength > 1 && result.digits[actualLength-1] == '0') {
        actualLength--;
    }
    result.length = actualLength;
    
    return result;
}

std::string BigHexInt::toString() const {
    std::string result;
    if (isNegative) {
        result += "-";
    }
    
    int msb = length - 1;
    while (msb > 0 && digits[msb] == '0') {
        msb--;
    }
    
    for (int i = msb; i >= 0; i--) {
        result += digits[i];
    }
    
    return result;
}

void BigHexInt::print() const {
    if (isNegative) {
        std::cout << "-";
    }
    
    int msb = HEX_SIZE - 1;
    while (msb > 0 && digits[msb] == '0') {
        msb--;
    }
    
    for (int i = msb; i >= 0; i--) {
        std::cout << digits[i];
    }
    std::cout << std::endl;
}

int BigHexInt::compare(const BigHexInt& other) const {
    if (isNegative && !other.isNegative) return -1;
    if (!isNegative && other.isNegative) return 1;
    
    for (int i = HEX_SIZE - 1; i >= 0; i--) {
        int digitA = convertHexDigitToInt(digits[i]);
        int digitB = convertHexDigitToInt(other.digits[i]);
        if (digitA != digitB) {
            if (isNegative)
                return (digitA > digitB) ? -1 : 1;
            else
                return (digitA > digitB) ? 1 : -1;
        }
    }
    return 0;
}

BigHexInt BigHexInt::operator+(const BigHexInt& other) const {
    if (isNegative != other.isNegative) {
        if (isNegative) {
            BigHexInt absA = *this;
            absA.isNegative = false;
            return other - absA;
        } else {
            BigHexInt absB = other;
            absB.isNegative = false;
            return *this - absB;
        }
    }
    
    BigHexInt result;
    std::fill(result.digits, result.digits + MAX_HEX_RESULT_SIZE, '0');
    int carry = 0;
    
    for (int i = 0; i < HEX_SIZE; i++) {
        int digitA = convertHexDigitToInt(digits[i]);
        int digitB = convertHexDigitToInt(other.digits[i]);
        int sum = digitA + digitB + carry;
        result.digits[i] = HEX_DIGIT_STR[sum % 16];
        carry = sum / 16;
    }
    
    result.isNegative = isNegative;
    
    int actualLength = HEX_SIZE;
    if (carry > 0 && actualLength < MAX_HEX_RESULT_SIZE) {
        result.digits[actualLength] = HEX_DIGIT_STR[carry];
        actualLength++;
    }
    
    while (actualLength > 1 && result.digits[actualLength-1] == '0') {
        actualLength--;
    }
    result.length = actualLength;
    
    return result;
}

BigHexInt BigHexInt::operator-(const BigHexInt& other) const {
    if (isNegative != other.isNegative) {
        BigHexInt absB = other;
        absB.isNegative = !other.isNegative;
        return *this + absB;
    }
    
    BigHexInt result;
    std::fill(result.digits, result.digits + MAX_HEX_RESULT_SIZE, '0');
    int borrow = 0;
    
    int cmp = 0;
    for (int i = HEX_SIZE - 1; i >= 0; i--) {
        int digitA = convertHexDigitToInt(digits[i]);
        int digitB = convertHexDigitToInt(other.digits[i]);
        if (digitA != digitB) {
            cmp = (digitA > digitB) ? 1 : -1;
            break;
        }
    }
    
    const BigHexInt *larger, *smaller;
    if (cmp >= 0) {
        larger = this;
        smaller = &other;
        result.isNegative = isNegative;
    } else {
        larger = &other;
        smaller = this;
        result.isNegative = !isNegative;
    }
    
    for (int i = 0; i < HEX_SIZE; i++) {
        int digitL = convertHexDigitToInt(larger->digits[i]);
        int digitS = convertHexDigitToInt(smaller->digits[i]);
        int diff = digitL - digitS - borrow;
        
        if (diff < 0) {
            diff += 16;
            borrow = 1;
        } else {
            borrow = 0;
        }
        
        result.digits[i] = HEX_DIGIT_STR[diff];
    }
    
    int actualLength = HEX_SIZE;
    while (actualLength > 1 && result.digits[actualLength-1] == '0') {
        actualLength--;
    }
    result.length = actualLength;
    
    return result;
}

BigHexInt BigHexInt::clone() const {
    BigHexInt result;
    std::fill(result.digits, result.digits + MAX_HEX_RESULT_SIZE, '0');
    for (int i = 0; i < length; i++) {
        result.digits[i] = digits[i];
    }
    result.length = length;
    result.isNegative = isNegative;
    return result;
}

void BigHexInt::shiftLeftInPlace(int n) {
    if (length + n > HEX_SIZE) {
        throw OverflowException("shift left operation");
    }
    
    for (int i = length - 1; i >= 0; i--) {
        if (i + n < HEX_SIZE) {
            digits[i + n] = digits[i];
        }
    }

    for (int i = 0; i < n; i++) {
        digits[i] = '0';
    }

    length = length + n;
    if (length > HEX_SIZE) {
        length = HEX_SIZE;
    }
}

BigHexInt BigHexInt::shiftLeft(int n) const {
    BigHexInt result = clone();
    result.shiftLeftInPlace(n);
    return result;
}

BigHexInt BigHexInt::getLower(int n) const {
    BigHexInt res;
    std::fill(res.digits, res.digits + MAX_HEX_RESULT_SIZE, '0');
    int actual = std::min(length, n);
    for (int i = 0; i < actual; i++) {
        res.digits[i] = digits[i];
    }
    res.length = (actual == 0) ? 1 : actual;
    res.isNegative = false;
    return res;
}

BigHexInt BigHexInt::getHigher(int n) const {
    BigHexInt res;
    std::fill(res.digits, res.digits + MAX_HEX_RESULT_SIZE, '0');
    if (length <= n) {
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
    res.isNegative = false;
    return res;
}

BigHexInt BigHexInt::pad(int targetLen) const {
    BigHexInt res = clone();
    if (res.length < targetLen) {
        for (int i = res.length; i < targetLen; i++) {
            res.digits[i] = '0';
        }
        res.length = targetLen;
    }
    return res;
}

BigHexInt BigHexInt::multiplyNaive(const BigHexInt& other) const {
    BigHexInt result;
    std::fill(result.digits, result.digits + MAX_HEX_RESULT_SIZE, '0');
    result.length = length + other.length;
    result.isNegative = isNegative != other.isNegative;

    if (result.length >= MAX_HEX_RESULT_SIZE) {
        throw OverflowException("naive multiplication");
    }

    for (int i = 0; i < length; i++) {
        int carry = 0;
        int a_digit = convertHexDigitToInt(digits[i]);
        for (int j = 0; j < other.length || carry; j++) {
            int b_digit = (j < other.length) ? convertHexDigitToInt(other.digits[j]) : 0;
            int current = convertHexDigitToInt(result.digits[i+j]);
            int prod = current + a_digit * b_digit + carry;
            result.digits[i+j] = HEX_DIGIT_STR[prod % 16];
            carry = prod / 16;
        }
    }

    while (result.length > 1 && result.digits[result.length-1] == '0') {
        result.length--;
    }
    return result;
}

BigHexInt BigHexInt::karatsuba(const BigHexInt& other) const {
    // Check if result is already memoized
    std::string thisStr = this->toString();
    std::string otherStr = other.toString();
    
    // Normalize the order for consistent memoization
    if (thisStr > otherStr) {
        std::swap(thisStr, otherStr);
    }
    
    std::pair<std::string, std::string> key = {thisStr, otherStr};
    
    // Check if we already computed this multiplication
    auto it = karatsubaMemo.find(key);
    if (it != karatsubaMemo.end()) {
        // Found in memoization table, create BigHexInt from stored result
        return BigHexInt::createFromString(it->second);
    }

    BigHexInt result;
    
    // Base cases
    if ((length == 1 && digits[0] == '0') ||
        (other.length == 1 && other.digits[0] == '0')) {
        BigHexInt zero;
        std::fill(zero.digits, zero.digits + MAX_HEX_RESULT_SIZE, '0');
        zero.length = 1;
        zero.isNegative = false;
        
        // Memoize the result
        karatsubaMemo[key] = zero.toString();
        return zero;
    }

    if (length <= KARATSUBA_THRESHOLD || other.length <= KARATSUBA_THRESHOLD) {
        result = multiplyNaive(other);
        // Memoize the result
        karatsubaMemo[key] = result.toString();
        return result;
    }

    int n = std::max(length, other.length);
    BigHexInt x = pad(n);
    BigHexInt y = other.pad(n);

    int m = n / 2;

    BigHexInt low1 = x.getLower(m);
    BigHexInt high1 = x.getHigher(m);
    BigHexInt low2 = y.getLower(m);
    BigHexInt high2 = y.getHigher(m);

    BigHexInt z0 = low1.karatsuba(low2);
    BigHexInt z2 = high1.karatsuba(high2);

    BigHexInt sum1 = low1 + high1;
    BigHexInt sum2 = low2 + high2;
    BigHexInt z1 = sum1.karatsuba(sum2);

    z1 = z1 - z2;
    z1 = z1 - z0;

    BigHexInt part1 = z2.shiftLeft(2 * m);
    BigHexInt part2 = z1.shiftLeft(m);
    BigHexInt temp = part1 + part2;
    result = temp + z0;

    while (result.length > 1 && result.digits[result.length-1] == '0') {
        result.length--;
    }
    
    // Memoize the result
    karatsubaMemo[key] = result.toString();
    return result;
}

BigHexInt BigHexInt::operator*(const BigHexInt& other) const {
    BigHexInt result;
    
    // Use Karatsuba for larger numbers (when combined length > 24)
    // if (length + other.length > 8) {
    //     result = karatsuba(other);
    // } else {
    //     result = multiplyNaive(other);
    // }
    result = karatsuba(other);
    result.isNegative = isNegative != other.isNegative;
    return result;
}

bool BigHexInt::isGreaterOrEqual(const BigHexInt& other) const {
    if (length != other.length) {
        return length > other.length;
    }
    
    for (int i = length - 1; i >= 0; i--) {
        int digitA = convertHexDigitToInt(digits[i]);
        int digitB = convertHexDigitToInt(other.digits[i]);
        if (digitA != digitB) {
            return digitA > digitB;
        }
    }
    return true;  // Equal
}

// BigHexInt BigHexInt::divide(const BigHexInt& divisor, BigHexInt* remainder) const {
//     if (divisor.isZero()) {
//         throw DivisionByZeroException();
//     }
    
//     BigHexInt quotient;
//     std::fill(quotient.digits, quotient.digits + MAX_HEX_RESULT_SIZE, '0');
//     quotient.length = 1;
//     quotient.isNegative = isNegative != divisor.isNegative;
    
//     int cmp = compare(divisor);
//     if (cmp == 0) {
//         quotient.digits[0] = '1';
//         if (remainder != nullptr) {
//             std::fill(remainder->digits, remainder->digits + MAX_HEX_RESULT_SIZE, '0');
//             remainder->length = 1;
//             remainder->isNegative = false;
//         }
//         return quotient;
//     } else if ((cmp < 0 && !isNegative && !divisor.isNegative) || 
//                (cmp > 0 && isNegative && divisor.isNegative)) {
//         quotient.digits[0] = '0';
//         if (remainder != nullptr) {
//             *remainder = *this;
//         }
//         return quotient;
//     }
    
//     // Simplified division - for full implementation, proper long division needed
//     return quotient;
// }

BigHexInt BigHexInt::divide(const BigHexInt& divisor, BigHexInt* remainder) const {
    if (divisor.isZero()) {
        throw DivisionByZeroException();
    }
    
    // Handle division by zero case
    if (this->isZero()) {
        BigHexInt zero;
        if (remainder != nullptr) {
            *remainder = zero;
        }
        return zero;
    }
    
    // Create absolute values for calculation
    BigHexInt dividend = *this;
    BigHexInt divisorAbs = divisor;
    dividend.isNegative = false;
    divisorAbs.isNegative = false;
    
    // Initialize quotient
    BigHexInt quotient;
    std::fill(quotient.digits, quotient.digits + MAX_HEX_RESULT_SIZE, '0');
    quotient.length = 1;
    quotient.isNegative = this->isNegative != divisor.isNegative;
    
    // Quick comparison checks
    int cmp = dividend.compare(divisorAbs);
    if (cmp == 0) {
        // dividend equals divisor
        quotient.digits[0] = '1';
        quotient.length = 1;
        if (remainder != nullptr) {
            std::fill(remainder->digits, remainder->digits + MAX_HEX_RESULT_SIZE, '0');
            remainder->length = 1;
            remainder->isNegative = false;
        }
        return quotient;
    } else if (cmp < 0) {
        // dividend < divisor
        quotient.digits[0] = '0';
        quotient.length = 1;
        quotient.isNegative = false;
        if (remainder != nullptr) {
            *remainder = *this;
        }
        return quotient;
    }
    
    // Perform long division
    BigHexInt currentDividend;
    std::fill(currentDividend.digits, currentDividend.digits + MAX_HEX_RESULT_SIZE, '0');
    currentDividend.length = 1;
    currentDividend.isNegative = false;
    
    std::fill(quotient.digits, quotient.digits + MAX_HEX_RESULT_SIZE, '0');
    int quotientPos = 0;
    
    // Process each digit of the dividend from left to right
    for (int i = dividend.length - 1; i >= 0; i--) {
        // Shift current dividend left and add next digit
        if (!currentDividend.isZero()) {
            // Shift left by one hex digit
            for (int j = currentDividend.length; j > 0; j--) {
                currentDividend.digits[j] = currentDividend.digits[j-1];
            }
            currentDividend.digits[0] = dividend.digits[i];
            currentDividend.length++;
        } else {
            // First non-zero digit
            currentDividend.digits[0] = dividend.digits[i];
            currentDividend.length = 1;
        }
        
        // Find how many times divisor goes into current dividend
        int count = 0;
        while (currentDividend.compare(divisorAbs) >= 0) {
            // Subtract divisor from current dividend
            BigHexInt temp = currentDividend - divisorAbs;
            currentDividend = temp;
            count++;
        }
        
        // Store the count in quotient
        if (count > 0 || quotientPos > 0) {
            quotient.digits[quotientPos] = convertIntToHexChar(count);
            quotientPos++;
        }
    }
    
    // Set quotient length and handle case where quotient is zero
    if (quotientPos == 0) {
        quotient.digits[0] = '0';
        quotient.length = 1;
        quotient.isNegative = false;
    } else {
        quotient.length = quotientPos;
        // Reverse the quotient digits since we built it backwards
        for (int i = 0; i < quotient.length / 2; i++) {
            char temp = quotient.digits[i];
            quotient.digits[i] = quotient.digits[quotient.length - 1 - i];
            quotient.digits[quotient.length - 1 - i] = temp;
        }
    }
    
    // Set remainder if requested
    if (remainder != nullptr) {
        *remainder = currentDividend;
        remainder->isNegative = this->isNegative && !remainder->isZero();
    }
    
    return quotient;
}

BigHexInt BigHexInt::operator/(const BigHexInt& other) const {
    return divide(other, nullptr);
}

BigHexInt BigHexInt::operator%(const BigHexInt& other) const {
    BigHexInt remainder;
    divide(other, &remainder);
    return remainder;
}

bool BigHexInt::isZero() const {
    for (int i = 0; i < length; i++) {
        if (digits[i] != '0') return false;
    }
    return true;
}

bool BigHexInt::isOne() const {
    if (length < 1) return false;
    if (digits[0] != '1') return false;
    for (int i = 1; i < length; i++) {
        if (digits[i] != '0') return false;
    }
    return true;
}

bool BigHexInt::isValidInput(const std::string& str) {
    if (str.empty()) return false;
    
    int start = 0;
    if (str[0] == '-') {
        start = 1;
        if (str.length() == 1) return false;
    }

    for (size_t i = start; i < str.length(); i++) {
        char c = str[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    return true;
}

void initializeLookupTable() {
    try {
        for (int i = 0; i < HEX_LOOKUP_SIZE; i++) {
            for (int j = 0; j < HEX_LOOKUP_SIZE; j++) {
                hexMultiplyLookup[i][j] = -1;
            }
        }

        std::ifstream file(LOOKUP_FILE);
        if (!file.is_open()) {
            std::cout << "Warning: Lookup file not found. Will create new one on exit." << std::endl;
            return;
        }

        int i_val, j_val, product_val;
        char colon1, colon2;
        while (file >> i_val >> colon1 >> j_val >> colon2 >> product_val) {
            if (i_val >= 0 && i_val < HEX_LOOKUP_SIZE && 
                j_val >= 0 && j_val < HEX_LOOKUP_SIZE) {
                hexMultiplyLookup[i_val][j_val] = product_val;
            }
        }
        file.close();
        std::cout << "Lookup table loaded successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing lookup table: " << e.what() << std::endl;
    }
}


void closeAndUpdateFile() {
    try {
        std::cout << "Updating memoization file..." << std::endl;
        
        std::ofstream file(LOOKUP_FILE, std::ios::app);
        if (!file.is_open()) {
            throw FileIOException(LOOKUP_FILE, "open for writing");
        }

        // Write new lookup table entries
        for (int i = 0; i < HEX_LOOKUP_SIZE; i++) {
            for (int j = 0; j < HEX_LOOKUP_SIZE; j++) {
                if (hexMultiplyLookup[i][j] != -1) {
                    file << i << ":" << j << ":" << hexMultiplyLookup[i][j] << std::endl;
                }
            }
        }

        // Write Karatsuba memoization results
        for (const auto& entry : karatsubaMemo) {
            file << "KARATSUBA:" << entry.first.first << ":" << entry.first.second 
                 << ":" << entry.second << std::endl;
        }

        file.close();
        std::cout << "Memoization file updated successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating memoization file: " << e.what() << std::endl;
    }
}

std::pair<std::string, std::string> getTwoValidNumbers() {
    std::string num1, num2;
    
    while (true) {
        std::cout << "Enter first number: ";
        std::cin >> num1;
        
        if (BigInt::isValidInput(num1) || BigHexInt::isValidInput(num1)) {
            break;
        } else {
            std::cout << "Invalid input. Please enter a valid decimal or hexadecimal number." << std::endl;
        }
    }
    
    while (true) {
        std::cout << "Enter second number: ";
        std::cin >> num2;
        
        if (BigInt::isValidInput(num2) || BigHexInt::isValidInput(num2)) {
            break;
        } else {
            std::cout << "Invalid input. Please enter a valid decimal or hexadecimal number." << std::endl;
        }
    }
    
    return std::make_pair(num1, num2);
}
// //Timer testing 
// void test_Bigdata_Hex(char operation)
// {
//    switch(operation)
//    {
//     case '+':
//         {
//             //open BigDataAdd.txt
//             //each line contains two Hexadecimal numbers seperated by';'
//             //read all these into std::vector<std::pair<std::string, std::string>> TestData
//             //Call a function that initialises our timer.
//             //Timer initialisation uis simply ' Timer t("Hexadecimal Addition: "); '
//             //The timer is custom written and it is Scope based Thats why calling a new function
//             //BigDataAdd contains many lines in the scale of 10,000
//             //and this code im using for benchmarking
//         }
//    }
//    //same goes for case "-", case"*""
// }

// int main() {
//     try {
//         std::atexit(closeAndUpdateFile);
//         initializeLookupTable();

//         bool isHex=true;
//         char hexchar;
//         std::cout<<"Input Y or y if the numbers are isHex"<<std::endl;
//         std::cin>>hexchar;

//         isHex = ( hexchar== 'Y' || hexchar == 'y');
//         int test_cases;
//         std::cin >> test_cases;
//         std::cin.ignore(); // Clear newline

//         for (int t = 0; t < test_cases; ++t) {
//             char op;
//             std::string num1, num2;

//             std::cin >> op;
//             std::cin >> num1 >> num2;


//             try {
//                 if (isHex) {
//                     BigHexInt a(num1), b(num2), result;
//                     switch (op) {
//                         case '+': 
//                         {
//                             Timer t("Hexadecimal Addition: ");
//                             result = a + b;
//                             break;
//                         }
//                         case '-': 
//                         {
//                             Timer t("Hexadecimal Subtraction");
//                             result = a - b;
//                             break;
//                         }

//                         case '*': result = a * b; break;
//                         case '/': result = a / b; break;
//                         case '%': result = a % b; break;
//                         default:
//                             std::cout << "Invalid operator: " << op << "\n";
//                             continue;
//                     }
//                     result.print();
//                 } else {
//                     BigInt a(num1), b(num2), result;
//                     switch (op) {
//                         case '+': result = a + b; break;
//                         case '-': result = a - b; break;
//                         case '*': result = a * b; break;
//                         case '/':
//                         case '%':
//                             std::cout << "Division/Modulo only supported for hexadecimal.\n";
//                             continue;
//                         default:
//                             std::cout << "Invalid operator: " << op << "\n";
//                             continue;
//                     }
//                     result.print();
//                 }
//             }
//             catch (const BigIntException& e) {
//                 std::cout << "Error: " << e.what() << "\n";
//             }
//         }
//     }
//     catch (const std::exception& e) {
//         std::cerr << "Fatal error: " << e.what() << "\n";
//         return 1;
//     }

//     return 0;
// }
BigHexInt BigHexInt::modPow(const BigHexInt& exponent, const BigHexInt& modulus) const {
    // Handle edge cases
    if (modulus.isZero()) {
        throw std::invalid_argument("Modulus cannot be zero");
    }
    
    if (modulus.isOne()) {
        return BigHexInt("0"); // Any number mod 1 is 0
    }
    
    if (exponent.isZero()) {
        return BigHexInt("1"); // Any number^0 is 1
    }
    
    if (this->isZero()) {
        return BigHexInt("0"); // 0^n is 0 for n > 0
    }
    
    // Handle negative exponents (not typically supported in modular arithmetic)
    if (exponent.isNegative) {
        throw std::invalid_argument("Negative exponents not supported in modular exponentiation");
    }
    
    // Convert base to positive and take mod
    BigHexInt base = *this;
    if (base.isNegative) {
        // Convert negative base to positive equivalent in modular arithmetic
        base.isNegative = false;
        BigHexInt temp = base % modulus;
        base = modulus - temp;
    } else {
        base = base % modulus;
    }
    
    if (base.isZero()) {
        return BigHexInt("0");
    }
    
    BigHexInt result("1");
    BigHexInt exp = exponent;
    
    // Binary exponentiation with modular reduction
    while (!exp.isZero()) {
        // If exponent is odd, multiply result by current base
        if (exp.isOdd()) {
            result = (result * base) % modulus;
        }
        
        // Square the base and halve the exponent
        base = (base * base) % modulus;
        exp = exp.divideByTwo();
    }
    
    return result;
}
bool BigHexInt::isOdd() const {
    if (length == 0) return false;
    
    // Check the least significant digit
    char lastDigit = digits[0];
    int lastDigitValue = convertHexDigitToInt(lastDigit);
    
    return (lastDigitValue & 1) == 1;
}

// Helper function to divide a BigHexInt by 2 (right shift by 1 bit)
BigHexInt BigHexInt::divideByTwo() const {
    if (isZero()) {
        return BigHexInt("0");
    }
    
    BigHexInt result;
    std::fill(result.digits, result.digits + MAX_HEX_RESULT_SIZE, '0');
    result.isNegative = isNegative;
    
    int carry = 0;
    int resultLength = 0;
    
    // Process digits from most significant to least significant
    for (int i = length - 1; i >= 0; i--) {
        int digitValue = convertHexDigitToInt(digits[i]);
        digitValue += carry * 16; // Add carry from previous position
        
        int newDigit = digitValue / 2;
        carry = digitValue % 2;
        
        if (newDigit > 0 || resultLength > 0) {
            result.digits[resultLength] = convertIntToHexChar(newDigit);
            resultLength++;
        }
    }
    
    if (resultLength == 0) {
        result.digits[0] = '0';
        result.length = 1;
        result.isNegative = false;
    } else {
        result.length = resultLength;
        // Reverse the result since we built it backwards
        for (int i = 0; i < result.length / 2; i++) {
            char temp = result.digits[i];
            result.digits[i] = result.digits[result.length - 1 - i];
            result.digits[result.length - 1 - i] = temp;
        }
    }
    
    return result;
}