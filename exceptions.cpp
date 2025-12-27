
#include "exceptions.hpp"


BigIntException::BigIntException(const std::string& msg) : message(msg) {}
const char* BigIntException::what() const noexcept  {
    return message.c_str();
}

DivisionByZeroException::DivisionByZeroException()
 : BigIntException("Division by zero is not allowed") {}

InvalidInputException::InvalidInputException(const std::string& input)
 : BigIntException("Invalid input: " + input) {}

OverflowException::OverflowException(const std::string& operation)
 : BigIntException("Overflow occurred during " + operation) {}

FileIOException::FileIOException(const std::string& filename, const std::string& operation)
 : BigIntException("File I/O error: Cannot " + operation + " file " + filename) {}
