#pragma once

#include<exception>
#include<string>

//main exception class  (acts as parent to all exceptions)
class BigIntException : public std::exception {
protected:
    std::string message;
public:
    BigIntException(const std::string& msg);
    virtual const char* what() const noexcept override;
};


//sub exceptions
class DivisionByZeroException : public BigIntException {
public:
    DivisionByZeroException();
};

class InvalidInputException : public BigIntException {
public:
    InvalidInputException(const std::string& input);
};

class OverflowException : public BigIntException {
public:
    OverflowException(const std::string& operation);
};

class FileIOException : public BigIntException {
public:
    FileIOException(const std::string& filename, const std::string& operation);
};