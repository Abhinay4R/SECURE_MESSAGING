# Secure Messaging Repository

This repository contains a C++ implementation of a custom **BigInteger** datatype designed for cryptographic applications. The core of this project is a robust library that supports arithmetic operations on arbitrarily large numbers, which is then used to implement a terminal-based **Diffie-Hellman Key Exchange** protocol.

## Features

### BigInteger Datatype Implementation

The project features two distinct `BigInteger` classes, `BigInt` and `BigHexInt`, to handle both decimal and hexadecimal representations of large numbers.

  * [cite\_start]**Arbitrary Precision:** The `BigInt` class uses a `char` array to store decimal digits, with a constant `MAX_DIGITS` set to 618, sufficient for numbers up to 2048 bits[cite: 1, 4].
  * [cite\_start]**Hexadecimal Support:** The `BigHexInt` class handles hexadecimal digits and is optimized for cryptographic operations, with a `HEX_SIZE` of 128 for 512-bit numbers[cite: 1, 4].
  * **Robust Arithmetic:** Both classes support fundamental arithmetic operations, including addition, subtraction, and multiplication. [cite\_start]`BigHexInt` extends this to include division and modulo operations[cite: 1].

### Optimized Karatsuba Multiplication

The multiplication of large numbers is a performance-critical operation. This project uses the Karatsuba algorithm to achieve better-than-naive time complexity.

  * [cite\_start]**Hybrid Approach:** A hybrid strategy is employed where a `KARATSUBA_THRESHOLD` of 8 is used to switch to a simpler naive multiplication algorithm for smaller numbers, avoiding the overhead of recursion for small inputs[cite: 1].
  * [cite\_start]**Dynamic Programming:** The Karatsuba implementation is optimized with a memoization table (`karatsubaMemo`) to store and reuse the results of sub-problems, significantly reducing redundant calculations and improving overall performance[cite: 1, 4].
  * [cite\_start]**Performance:** This optimization results in a highly efficient multiplication algorithm, achieving an average of 530 nanoseconds for 100,000 multiplications[cite: 5].

### Diffie-Hellman Key Exchange

A console-based application demonstrates the practical use of the `BigHexInt` class for cryptographic key exchange.

  * [cite\_start]**1024-bit Prime Generation:** The protocol uses 1024-bit primes, which are generated using the Miller-Rabin primality test to ensure security[cite: 6].
  * [cite\_start]**Modular Exponentiation:** The key exchange relies on the `modPower` function for efficient modular exponentiation (`base^exponent % modulus`), a cornerstone of modern public-key cryptography[cite: 1].

### Technical Details & Implementation Nitpicks

  * [cite\_start]**Digit Storage:** The digits of the large numbers are stored in a `char` array in reverse order, with the least significant digit at index 0. This simplifies the implementation of basic arithmetic operations like addition and subtraction[cite: 1].
  * [cite\_start]**Custom Exception Handling:** The code includes a robust error handling system with custom exception classes such as `DivisionByZeroException`, `InvalidInputException`, and `OverflowException` to provide clear and informative error messages[cite: 1, 5].
  * **Random Number Generation:** The Miller-Rabin primality test relies on a random number generator seeded by `std::random_device` and `std::mt19937_64` for a strong source of entropy. [cite\_start]A simplified helper function, `generateRandomBigHexIntInRange`, is used for generating random numbers within a specific range[cite: 1].
  * [cite\_start]**Division Algorithm:** The `BigHexInt` division operator is implemented using a classic schoolbook long division method, providing a straightforward and reliable way to handle the operation[cite: 1].

## Technologies Used

  * [cite\_start]**C++:** The core programming language for the entire project[cite: 4].
  * [cite\_start]**Dynamic Programming:** Applied to optimize the Karatsuba multiplication algorithm[cite: 4].

## Getting Started

### Prerequisites

You need a C++ compiler (e.g., g++ or clang) to compile and run the code.

### Installation and Compilation

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/secure-messaging-repo.git
    cd secure-messaging-repo
    ```
2.  **Compile the source code:**
    ```bash
    g++ -o secure_messaging BigIntv1.cpp -std=c++17
    ```

### Usage

1.  **Run the compiled executable:**
    ```bash
    ./secure_messaging
    ```
2.  Follow the on-screen prompts to perform various operations, such as BigInt arithmetic or the Diffie-Hellman key exchange.

## License

You are open to use this, feel free to update this according to your needs.


## Contact

If you have any questions or feedback, feel free to reach out:

  * **Name:** Abhinay Ragam
  * [cite\_start]**Email:** [abhinayragam@gmail.com](mailto:abhinayragam@gmail.com) 
  * **LinkedIn:** [https://www.linkedin.com/in/abhinay-ragam-1a7a99295/]

-----
