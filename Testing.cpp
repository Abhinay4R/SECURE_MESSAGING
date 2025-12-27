#include "Testing.hpp"
#include "Timer.hpp"
#include "BigInt.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <utility>

void test_Bigdata_Hex(char operation)
{
    std::string filename;
    std::string benchmarkLabel;

    switch (operation)
    {
        case '+':
            filename = "BigDataHexAdd";
            benchmarkLabel = "Hexadecimal Addition: ";
            break;
        case '-':
            filename = "BigDataHexSub";
            benchmarkLabel = "Hexadecimal Subtraction: ";
            break;
        case '*':
            filename = "BigDataHexMul";
            benchmarkLabel = "Hexadecimal Multiplication: ";
            break;
        default:
            std::cerr << "Unsupported operation: " << operation << "\n";
            return;
    }

    std::ifstream infile(filename);
    if (!infile)
    {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    std::vector<std::pair<std::string, std::string>> TestData;
    std::string line;

    while (std::getline(infile, line))
    {
        std::stringstream ss(line);
        std::string hex1, hex2;

        if (std::getline(ss, hex1, ';') && std::getline(ss, hex2))
        {
            TestData.emplace_back(hex1, hex2);
        }
    }

    infile.close();

    // Scope-based timer
    Timer t(benchmarkLabel);

    // Execute all operations
   for (const auto& pair : TestData)
{
    const std::string& hex1 = pair.first;
    const std::string& hex2 = pair.second;

    BigHexInt num1(hex1);
    BigHexInt num2(hex2);
    BigHexInt result("0");

    switch (operation)
    {
        case '+':
            result = num1 + num2;
            // result.print();
            break;
        case '-':
            result = num1 - num2;
            // result.print();
            break;
        case '*':
            result= num1 * num2;
            // result.print();
            break;
        default:
            std::cout<<" UNSUPPORTED OPERATION"<<std::endl;
    }
}
}

void test_Bigdata_Deci(char operation)
{
    std::string filename;
    std::string benchmarkLabel;

    switch (operation)
    {
        case '+':
            filename = "BigDataDeciAdd";
            benchmarkLabel = "decimal Addition: ";
            break;
        case '-':
            filename = "BigDataDeciSub";
            benchmarkLabel = "decimal Subtraction: ";
            break;
        case '*':
            filename = "BigDataDeciMul";
            benchmarkLabel = "decimal Multiplication: ";
            break;
        default:
            std::cerr << "Unsupported operation: " << operation << "\n";
            return;
    }

    std::ifstream infile(filename);
    if (!infile)
    {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    std::vector<std::pair<std::string, std::string>> TestData;
    std::string line;

    while (std::getline(infile, line))
    {
        std::stringstream ss(line);
        std::string hex1, hex2;

        if (std::getline(ss, hex1, ';') && std::getline(ss, hex2))
        {
            TestData.emplace_back(hex1, hex2);
        }
    }

    infile.close();

    // Scope-based timer
    Timer t(benchmarkLabel);

    // Execute all operations
   for (const auto& pair : TestData)
    {
    const std::string& hex1 = pair.first;
    const std::string& hex2 = pair.second;

    BigInt num1(hex1);
    BigInt num2(hex2);
    BigInt result("0");

    switch (operation)
    {
        case '+':
            result = num1 + num2;
            // result.print();
            break;
        case '-':
            result = num1 - num2;
            // result.print();
            break;
        case '*':
            result = num1 * num2;
            // result.print();
            break;
    }
    }
}
