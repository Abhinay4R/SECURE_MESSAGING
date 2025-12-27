#pragma once

//libraries included
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <stdexcept>

//constants declared
constexpr const char* LOOKUP_FILE = "numberstorage";
constexpr const char* HEX_DIGIT_STR = "0123456789abcdef";
constexpr int MAX_DIGITS = 618;
constexpr int HEX_SIZE = 64;
constexpr int MAX_HEX_RESULT_SIZE = 128;
constexpr int HEX_LOOKUP_SIZE = 256;
constexpr int MAX_BINARY_SIZE = 1024;
constexpr int MAX_BINARY_RESULT_SIZE = 2048;
constexpr int KARATSUBA_THRESHOLD = 4;

// Global memoization map for Karatsuba multiplication
extern std::map<std::pair<std::string, std::string>, std::string> karatsubaMemo;

// Global lookup table for isHex multiplication
extern int hexMultiplyLookup[HEX_LOOKUP_SIZE][HEX_LOOKUP_SIZE];

// Utility functions
int convertHexDigitToInt(char c);
char convertIntToHexChar(int n);
void initializeLookupTable();
void closeAndUpdateFile();
std::pair<std::string, std::string> getTwoValidNumbers();




//class declarations
/*<----------------- BIG INT CLASS ------------------>*/
class BigInt {
public:
    char digits[MAX_DIGITS];
    int length;
    bool isNegative;

    BigInt();       
    BigInt(const std::string& str);

    static BigInt createFromString(const std::string& str);
    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    int compare(const BigInt& other) const;
    void print() const;
    static bool isValidInput(const std::string& str);
};




/*<---------------------BIG HEX INT CLASS---------------------->*/
class BigHexInt {
public:
    char digits[MAX_HEX_RESULT_SIZE];
    int length;
    bool isNegative;

    BigHexInt();
    BigHexInt(const std::string& str);

    static BigHexInt createFromString(const std::string& str);
    BigHexInt operator+(const BigHexInt& other) const;
    BigHexInt operator-(const BigHexInt& other) const;
    BigHexInt operator*(const BigHexInt& other) const;
    BigHexInt operator/(const BigHexInt& other) const;
    BigHexInt operator%(const BigHexInt& other) const;
    
    int compare(const BigHexInt& other) const;
    void print() const;
    static bool isValidInput(const std::string& str);
    
    // Helper methods
    BigHexInt clone() const;
    BigHexInt shiftLeft(int n) const;
    void shiftLeftInPlace(int n);
    BigHexInt getLower(int n) const;
    BigHexInt getHigher(int n) const;
    BigHexInt pad(int targetLen) const;
    bool isZero() const;
    bool isOne() const;
    bool isGreaterOrEqual(const BigHexInt& other) const;
    std::string toString() const;
    BigHexInt modPow(const BigHexInt& exponent, const BigHexInt& modulus) const;

private:
    bool isOdd() const;
    BigHexInt divideByTwo() const;
    BigHexInt multiplyNaive(const BigHexInt& other) const;
    BigHexInt karatsuba(const BigHexInt& other) const;
    BigHexInt divide(const BigHexInt& divisor, BigHexInt* remainder = nullptr) const;
};


