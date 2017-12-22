#ifndef __IO_HPP__
#define __IO_HPP__

#include <utility>
#include "matrix.hpp"

std::ostream& operator<<(std::ostream& stream, const FUOver2& val);
std::istream& operator>>(std::istream& stream, FUOver2& val);
std::ostream& operator<<(std::ostream& stream, const std::pair<FUMatrix, unsigned int>& val);
std::istream& operator>>(std::istream& stream, std::pair<FUMatrix, unsigned int>& val);

#endif // __IO_HPP__
