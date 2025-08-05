
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>
#include <iomanip>
#include <random>    // For std::random_device, std::mt19937, std::uniform_int_distribution
#include <algorithm> // For std::shuffle

// --- Verhoeff-Gumm Tables (Copied from your C code) ---
// Dihedral permutation table
static const int permutation_table[8][10] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
    {1, 5, 7, 6, 2, 8, 3, 0, 9, 4},
    {5, 8, 0, 3, 7, 9, 6, 1, 4, 2},
    {8, 9, 1, 6, 0, 4, 3, 5, 2, 7},
    {9, 4, 5, 3, 1, 2, 6, 8, 7, 0},
    {4, 2, 8, 6, 5, 7, 3, 9, 0, 1},
    {2, 7, 9, 3, 8, 0, 6, 4, 1, 5},
    {7, 0, 4, 6, 9, 1, 3, 2, 5, 8}
};

// Non-commutative multiplication table (D5 group)
static const int d5_mult_table[10][10] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
    {1, 2, 3, 4, 0, 6, 7, 8, 9, 5},
    {2, 3, 4, 0, 1, 7, 8, 9, 5, 6},
    {3, 4, 0, 1, 2, 8, 9, 5, 6, 7},
    {4, 0, 1, 2, 3, 9, 5, 6, 7, 8},
    {5, 9, 8, 7, 6, 0, 4, 3, 2, 1},
    {6, 5, 9, 8, 7, 1, 0, 4, 3, 2},
    {7, 6, 5, 9, 8, 2, 1, 0, 4, 3},
    {8, 7, 6, 5, 9, 3, 2, 1, 0, 4},
    {9, 8, 7, 6, 5, 4, 3, 2, 1, 0}
};

// Inverse lookup table
static const int inv_table[10] = {0, 4, 3, 2, 1, 5, 9, 8, 7, 6};

// Implementation of f(a,b) for Verhoeff-Gumm
static int verhoeff_f(int a, int b) {
    return d5_mult_table[a][permutation_table[0][b]];
}

// Verhoeff-Gumm check digit calculation for a 5-digit number
int calculate_verhoeff_check_digit(int five_digit_number) {
    if (five_digit_number < 0 || five_digit_number > 99999) {
        // Should not happen with controlled input for this benchmark
        return -1;
    }

    int a1 = (five_digit_number / 10000) % 10;
    int a2 = (five_digit_number / 1000) % 10;
    int a3 = (five_digit_number / 100) % 10;
    int a4 = (five_digit_number / 10) % 10;
    int a5 = five_digit_number % 10;

    // Calculate c = f(a1, f(a2, f(a3, f(a4, a5))))
    int c = verhoeff_f(a1, verhoeff_f(a2, verhoeff_f(a3, verhoeff_f(a4, a5))));

    // Return inverse of c
    return inv_table[c];
}

// --- Your Other Algorithms ---

// Helper to get digits from a number
std::vector<int> get_digits(int number, int num_digits) {
    std::string s = std::to_string(number);
    // Pad with leading zeros if necessary
    while (s.length() < num_digits) {
        s = "0" + s;
    }
    std::vector<int> digits;
    for (char c : s) {
        digits.push_back(c - '0');
    }
    return digits;
}

// Algorithm 1: Sum of first five digits % 10
int calculate_algo1_check_digit(int five_digit_number) {
    std::vector<int> digits = get_digits(five_digit_number, 5);
    int sum = 0;
    for (int digit : digits) {
        sum += digit;
    }
    return sum % 10;
}

// Algorithm 2: Sum of squares of first five digits % 10
int calculate_algo2_check_digit(int five_digit_number) {
    std::vector<int> digits = get_digits(five_digit_number, 5);
    int sum_sq = 0;
    for (int digit : digits) {
        sum_sq += digit * digit;
    }
    return sum_sq % 10;
}

// Algorithm 3: f = (a*b + b*c + c*d + d*e + e*a) % 10
int calculate_algo3_check_digit(int five_digit_number) {
    std::vector<int> digits = get_digits(five_digit_number, 5);
    if (digits.size() < 5) return -1; // Should not happen with valid input range

    int a = digits[0];
    int b = digits[1];
    int c = digits[2];
    int d = digits[3];
    int e = digits[4];

    return (a*b + b*c + c*d + d*e + e*a) % 10;
}

// Algorithm 4: f = (a*b*c + b*c*d + c*d*e + d*e*a + e*a*b) % 10
int calculate_algo4_check_digit(int five_digit_number) {
    std::vector<int> digits = get_digits(five_digit_number, 5);
    if (digits.size() < 5) return -1; // Should not happen with valid input range

    int a = digits[0];
    int b = digits[1];
    int c = digits[2];
    int d = digits[3];
    int e = digits[4];

    return (a*b*c + b*c*d + c*d*e + d*e*a + e*a*b) % 10;
}

// --- Luhn Algorithm ---
// Luhn Algorithm Check Digit Calculation for a 5-digit number
int calculate_luhn_check_digit(int five_digit_number) {
    std::vector<int> digits = get_digits(five_digit_number, 5);

    int sum = 0;
    bool double_digit = true; // Start doubling from the rightmost digit of the 5-digit number

    // Process from right to left (simulating the way Luhn processes full number)
    for (int i = digits.size() - 1; i >= 0; --i) {
        int digit = digits[i];
        if (double_digit) {
            digit *= 2;
            if (digit > 9) {
                digit -= 9;
            }
        }
        sum += digit;
        double_digit = !double_digit;
    }

    int check_digit = (sum * 9) % 10; // (10 - (sum % 10)) % 10 if sum % 10 is not 0
    return check_digit;
}

// Validation for Luhn
bool is_valid_luhn(int six_digit_number) {
    std::vector<int> digits = get_digits(six_digit_number, 6);

    int sum = 0;
    bool double_digit = false; // For full number, start doubling from the second to last digit

    for (int i = digits.size() - 1; i >= 0; --i) {
        int digit = digits[i];
        if (double_digit) {
            digit *= 2;
            if (digit > 9) {
                digit -= 9;
            }
        }
        sum += digit;
        double_digit = !double_digit;
    }
    return (sum % 10 == 0);
}

// --- Damm Algorithm ---
// Example Damm Quasigroup Table
static const int damm_table[10][10] = {
    {0, 3, 1, 7, 5, 9, 8, 6, 4, 2},
    {7, 0, 9, 2, 1, 5, 4, 8, 6, 3},
    {4, 2, 0, 6, 8, 7, 3, 9, 1, 5},
    {1, 7, 5, 0, 9, 8, 3, 4, 2, 6},
    {6, 1, 2, 3, 0, 4, 5, 7, 9, 8},
    {3, 6, 4, 9, 7, 0, 2, 5, 8, 1},
    {5, 8, 6, 4, 2, 1, 0, 3, 7, 9},
    {8, 9, 3, 1, 4, 2, 7, 0, 5, 6},
    {9, 4, 7, 8, 3, 6, 1, 0, 2, 5},
    {2, 5, 8, 0, 6, 3, 4, 1, 9, 7}
};

// Damm Algorithm Check Digit Calculation for a 5-digit number
int calculate_damm_check_digit(int five_digit_number) {
    std::vector<int> digits = get_digits(five_digit_number, 5);
    
    int interim_digit = 0;
    for (int digit : digits) {
        interim_digit = damm_table[interim_digit][digit];
    }
    return interim_digit; // The result of the final operation is the check digit
}

// Validation for Damm
bool is_valid_damm(int six_digit_number) {
    std::vector<int> digits = get_digits(six_digit_number, 6);

    int interim_digit = 0;
    for (int digit : digits) {
        interim_digit = damm_table[interim_digit][digit];
    }
    return (interim_digit == 0); // A valid Damm number will result in 0
}


// --- Validation Functions (for each algorithm) ---

// Checks if a 6-digit number is valid according to Verhoeff-Gumm
bool is_valid_verhoeff(int six_digit_number) {
    int five_digit_prefix = six_digit_number / 10;
    int actual_check_digit = six_digit_number % 10;
    return calculate_verhoeff_check_digit(five_digit_prefix) == actual_check_digit;
}

// Checks if a 6-digit number is valid according to Algorithm 1
bool is_valid_algo1(int six_digit_number) {
    int five_digit_prefix = six_digit_number / 10;
    int actual_check_digit = six_digit_number % 10;
    return calculate_algo1_check_digit(five_digit_prefix) == actual_check_digit;
}

// Checks if a 6-digit number is valid according to Algorithm 2
bool is_valid_algo2(int six_digit_number) {
    int five_digit_prefix = six_digit_number / 10;
    int actual_check_digit = six_digit_number % 10;
    return calculate_algo2_check_digit(five_digit_prefix) == actual_check_digit;
}

// Checks if a 6-digit number is valid according to Algorithm 3
bool is_valid_algo3(int six_digit_number) {
    int five_digit_prefix = six_digit_number / 10;
    int actual_check_digit = six_digit_number % 10;
    return calculate_algo3_check_digit(five_digit_prefix) == actual_check_digit;
}

// Checks if a 6-digit number is valid according to Algorithm 4
bool is_valid_algo4(int six_digit_number) {
    int five_digit_prefix = six_digit_number / 10;
    int actual_check_digit = six_digit_number % 10;
    return calculate_algo4_check_digit(five_digit_prefix) == actual_check_digit;
}

// --- Error Introduction Functions ---

// Introduces a single-digit substitution error
int introduce_substitution_error(int number, std::mt19937& gen, std::uniform_int_distribution<int>& dist_pos, std::uniform_int_distribution<int>& dist_digit) {
    std::string s = std::to_string(number);
    while (s.length() < 6) { // Ensure it's always 6 digits, e.g., 098765
        s = "0" + s;
    }
    int pos = dist_pos(gen); // Random position to change (0 to 5)
    int original_digit = s[pos] - '0';
    int new_digit;
    do {
        new_digit = dist_digit(gen); // Random new digit (0 to 9)
    } while (new_digit == original_digit); // Ensure it's a different digit

    s[pos] = (char)('0' + new_digit);
    return std::stoi(s);
}

// Introduces an adjacent transposition error
int introduce_transposition_error(int number, std::mt19937& gen, std::uniform_int_distribution<int>& dist_trans_pos) {
    std::string s = std::to_string(number);
    while (s.length() < 6) {
        s = "0" + s;
    }
    if (s.length() < 2) return number; // Cannot transpose if less than 2 digits

    int pos = dist_trans_pos(gen); // Random starting position for transposition (0 to length-2)
    std::swap(s[pos], s[pos + 1]);
    return std::stoi(s);
}


int main() {
    std::cout << "Benchmarking check digit algorithms for error detection...\n";

    // Random number generation setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist_six_digit_prefix(10000, 99999); // For first five digits
    std::uniform_int_distribution<> dist_pos(0, 5); // For substitution error position (0-indexed for 6 digits)
    std::uniform_int_distribution<> dist_digit(0, 9); // For new digit in substitution
    std::uniform_int_distribution<> dist_trans_pos(0, 4); // For transposition error position (0 to 4 for a 6-digit number)

    const int NUM_VALID_NUMBERS_TO_GENERATE = 10000; // Number of unique valid numbers to test
    const int NUM_ERRORS_PER_VALID_NUMBER = 50; // Number of errors to introduce for each valid number

    long long total_substitution_errors = 0;
    long long total_transposition_errors = 0;

    long long verhoeff_undetected_substitutions = 0;
    long long algo1_undetected_substitutions = 0;
    long long algo2_undetected_substitutions = 0;
    long long algo3_undetected_substitutions = 0;
    long long algo4_undetected_substitutions = 0;


    long long verhoeff_undetected_transpositions = 0;
    long long algo1_undetected_transpositions = 0;
    long long algo2_undetected_transpositions = 0;
    long long algo3_undetected_transpositions = 0;
    long long algo4_undetected_transpositions = 0;



    // To store unique valid prefixes to avoid duplicate testing and ensure randomization
    std::vector<int> valid_prefixes;
    for (int i = 10000; i <= 99999; ++i) {
        valid_prefixes.push_back(i);
    }
    std::shuffle(valid_prefixes.begin(), valid_prefixes.end(), gen); // Shuffle to pick random prefixes

    std::cout << "Generating " << NUM_VALID_NUMBERS_TO_GENERATE << " valid numbers and introducing "
              << NUM_ERRORS_PER_VALID_NUMBER << " errors for each...\n";

    for (int k = 0; k < NUM_VALID_NUMBERS_TO_GENERATE; ++k) {
        if (k % 1000 == 0) {
            std::cout << "Processing batch " << k / 1000 + 1 << "...\n";
        }

        int five_digit_prefix = valid_prefixes[k];

        // --- Generate valid numbers for each algorithm ---
        int valid_verhoeff_num = five_digit_prefix * 10 + calculate_verhoeff_check_digit(five_digit_prefix);
        int valid_algo1_num = five_digit_prefix * 10 + calculate_algo1_check_digit(five_digit_prefix);
        int valid_algo2_num = five_digit_prefix * 10 + calculate_algo2_check_digit(five_digit_prefix);
        int valid_algo3_num = five_digit_prefix * 10 + calculate_algo3_check_digit(five_digit_prefix);
        int valid_algo4_num = five_digit_prefix * 10 + calculate_algo4_check_digit(five_digit_prefix);
        int valid_luhn_num = five_digit_prefix * 10 + calculate_luhn_check_digit(five_digit_prefix); // Added Luhn
        int valid_damm_num = five_digit_prefix * 10 + calculate_damm_check_digit(five_digit_prefix); // Added Damm


        for (int j = 0; j < NUM_ERRORS_PER_VALID_NUMBER; ++j) {
            // Test Substitution Errors
            int erroneous_verhoeff_sub = introduce_substitution_error(valid_verhoeff_num, gen, dist_pos, dist_digit);
            if (is_valid_verhoeff(erroneous_verhoeff_sub)) {
                verhoeff_undetected_substitutions++;
            }
            int erroneous_algo1_sub = introduce_substitution_error(valid_algo1_num, gen, dist_pos, dist_digit);
            if (is_valid_algo1(erroneous_algo1_sub)) {
                algo1_undetected_substitutions++;
            }
            int erroneous_algo2_sub = introduce_substitution_error(valid_algo2_num, gen, dist_pos, dist_digit);
            if (is_valid_algo2(erroneous_algo2_sub)) {
                algo2_undetected_substitutions++;
            }
            int erroneous_algo3_sub = introduce_substitution_error(valid_algo3_num, gen, dist_pos, dist_digit);
            if (is_valid_algo3(erroneous_algo3_sub)) {
                algo3_undetected_substitutions++;
            }
            int erroneous_algo4_sub = introduce_substitution_error(valid_algo4_num, gen, dist_pos, dist_digit);
            if (is_valid_algo4(erroneous_algo4_sub)) {
                algo4_undetected_substitutions++;
            }
            total_substitution_errors++;

            // Test Transposition Errors
            int erroneous_verhoeff_trans = introduce_transposition_error(valid_verhoeff_num, gen, dist_trans_pos);
            if (is_valid_verhoeff(erroneous_verhoeff_trans)) {
                verhoeff_undetected_transpositions++;
            }
            int erroneous_algo1_trans = introduce_transposition_error(valid_algo1_num, gen, dist_trans_pos);
            if (is_valid_algo1(erroneous_algo1_trans)) {
                algo1_undetected_transpositions++;
            }
            int erroneous_algo2_trans = introduce_transposition_error(valid_algo2_num, gen, dist_trans_pos);
            if (is_valid_algo2(erroneous_algo2_trans)) {
                algo2_undetected_transpositions++;
            }
            int erroneous_algo3_trans = introduce_transposition_error(valid_algo3_num, gen, dist_trans_pos);
            if (is_valid_algo3(erroneous_algo3_trans)) {
                algo3_undetected_transpositions++;
            }
            int erroneous_algo4_trans = introduce_transposition_error(valid_algo4_num, gen, dist_trans_pos);
            if (is_valid_algo4(erroneous_algo4_trans)) {
                algo4_undetected_transpositions++;
            }

            total_transposition_errors++;
        }
    }

    std::cout << "\n--- False Positive Results (Error Detection) ---\n";
    std::cout << std::fixed << std::setprecision(5);

    std::cout << "\nTotal Substitution Errors Tested: " << total_substitution_errors << "\n";
    std::cout << "Algorithm                  | Undetected Substitutions | False Positive Rate (%)\n";
    std::cout << "---------------------------------------------------------------------------\n";
    std::cout << "Verhoeff-Gumm              | " << std::setw(24) << verhoeff_undetected_substitutions << " | "
              << (static_cast<double>(verhoeff_undetected_substitutions) / total_substitution_errors) * 100 << "\n";
    std::cout << "Algorithm 1 (Sum)          | " << std::setw(24) << algo1_undetected_substitutions << " | "
              << (static_cast<double>(algo1_undetected_substitutions) / total_substitution_errors) * 100 << "\n";
    std::cout << "Algorithm 2 (Sum of Squares)| " << std::setw(24) << algo2_undetected_substitutions << " | "
              << (static_cast<double>(algo2_undetected_substitutions) / total_substitution_errors) * 100 << "\n";
    std::cout << "Algorithm 3 (Paired Products)| " << std::setw(24) << algo3_undetected_substitutions << " | "
              << (static_cast<double>(algo3_undetected_substitutions) / total_substitution_errors) * 100 << "\n";
    std::cout << "Algorithm 4 (Triple Products)| " << std::setw(24) << algo4_undetected_substitutions << " | "
              << (static_cast<double>(algo4_undetected_substitutions) / total_substitution_errors) * 100 << "\n";

    std::cout << "\nTotal Transposition Errors Tested: " << total_transposition_errors << "\n";
    std::cout << "Algorithm                  | Undetected Transpositions | False Positive Rate (%)\n";
    std::cout << "-----------------------------------------------------------------------------\n";
    std::cout << "Verhoeff-Gumm              | " << std::setw(27) << verhoeff_undetected_transpositions << " | "
              << (static_cast<double>(verhoeff_undetected_transpositions) / total_transposition_errors) * 100 << "\n";
    std::cout << "Algorithm 1 (Sum)          | " << std::setw(27) << algo1_undetected_transpositions << " | "
              << (static_cast<double>(algo1_undetected_transpositions) / total_transposition_errors) * 100 << "\n";
    std::cout << "Algorithm 2 (Sum of Squares)| " << std::setw(27) << algo2_undetected_transpositions << " | "
              << (static_cast<double>(algo2_undetected_transpositions) / total_transposition_errors) * 100 << "\n";
    std::cout << "Algorithm 3 (Paired Products)| " << std::setw(27) << algo3_undetected_transpositions << " | "
              << (static_cast<double>(algo3_undetected_transpositions) / total_transposition_errors) * 100 << "\n";
    std::cout << "Algorithm 4 (Triple Products)| " << std::setw(27) << algo4_undetected_transpositions << " | "
              << (static_cast<double>(algo4_undetected_transpositions) / total_transposition_errors) * 100 << "\n";
    return 0;
}
