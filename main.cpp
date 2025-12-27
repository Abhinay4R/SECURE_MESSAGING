
#include "Bigint.hpp"
#include "exceptions.hpp"
#include "Timer.hpp"
#include "Testing.hpp"

int main() {
    try {
        std::atexit(closeAndUpdateFile);
        initializeLookupTable();
        bool testmode=false;
        bool isHex=true;
        char hexchar;
        char testchar;
        std::cout<<"Do you to test or Benchmark Code, if Yes press Y or y"<<std::endl;
        std::cin>>testchar;
        testmode= (testchar=='Y'||testchar=='y');
        if(testmode)
        {
            char op;
            std::cout<<"Input Y or y if the numbers are isHex"<<std::endl;
            std::cin>>hexchar;
            isHex = ( hexchar== 'Y' || hexchar == 'y');
            std::cin >> op;
            if(isHex)test_Bigdata_Hex(op);
            else test_Bigdata_Deci(op);
            return 0;
        }
        std::cout<<"Input Y or y if the numbers are isHex"<<std::endl;
        std::cin>>hexchar;

        isHex = ( hexchar== 'Y' || hexchar == 'y');
        int test_cases;
        std::cin >> test_cases;
        std::cin.ignore(); // Clear newline

        for (int t = 0; t < test_cases; ++t) {
            char op;
            std::string num1, num2;

            std::cin >> op;
            std::cin >> num1 >> num2;


            try {
                if (isHex) {
                    BigHexInt a(num1), b(num2), result;
                    switch (op) {
                        case '+': 
                        {
                            //Timer t("Hexadecimal Addition: ");
                            result = a + b;
                            break;
                        }
                        case '-': 
                        {
                            //Timer t("Hexadecimal Subtraction");
                            result = a - b;
                            break;
                        }

                        case '*': result = a * b; break;
                        case '/': result = a / b; break;
                        case '%': result = a % b; break;
                        default:
                            std::cout << "Invalid operator: " << op << "\n";
                            continue;
                    }
                    result.print();
                } else {
                    BigInt a(num1), b(num2), result;
                    switch (op) {
                        case '+': result = a + b; break;
                        case '-': result = a - b; break;
                        case '*': result = a * b; break;
                        case '/':
                        case '%':
                            std::cout << "Division/Modulo only supported for hexadecimal.\n";
                            continue;
                        default:
                            std::cout << "Invalid operator: " << op << "\n";
                            continue;
                    }
                    result.print();
                }
            }
            catch (const BigIntException& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
