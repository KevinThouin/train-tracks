#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include "arrow.hpp"

#include <cassert>
#include <iostream>
#include <list>
#include <algorithm>
#include <list>

class FUOver2 {
	unsigned char m_val;
	
	FUOver2(unsigned char val) : m_val(val) {}
	
public:
	FUOver2() : m_val(0) {}
	FUOver2(bool U, bool cst) : m_val((static_cast<unsigned char>(U)<<1) + static_cast<unsigned char>(cst)) {}
	
	FUOver2  operator+ (const FUOver2& other) const {return FUOver2(m_val ^ other.m_val);}
	FUOver2& operator+=(const FUOver2& other) {m_val ^= other.m_val; return *this;}
	FUOver2  operator* (const FUOver2& other) const {return FUOver2((m_val*other.m_val) & 0x03);}
	FUOver2& operator*=(const FUOver2& other) {m_val*=other.m_val; m_val&=0x03; return *this;}
	bool     operator==(const FUOver2& other) const {return m_val==other.m_val;}
	bool     operator!=(const FUOver2& other) const {return m_val!=other.m_val;}
	
	bool getU() const   {return (m_val&0x02) != 0;}
	bool getCst() const {return (m_val&0x01) != 0;}
};

class FUMatrix {
	unsigned int m_size = 0;
	FUOver2* m_data = nullptr;
	
public:
	FUMatrix() = default;
	FUMatrix(unsigned int size) : m_size(size), m_data(new FUOver2[size*size]()) {}
	FUMatrix(const FUMatrix& other) : m_size(other.m_size) {
		m_data = new FUOver2[m_size*m_size];
		std::copy(other.m_data, other.m_data+m_size*m_size, m_data);
	}
	FUMatrix(FUMatrix&& other) : m_size(other.m_size), m_data(other.m_data) {
		other.m_size = 0;
		other.m_data = nullptr;
	}
	~FUMatrix() {delete [] m_data;}
	
	unsigned int size() const {return m_size;}
	
	FUMatrix  operator*(const FUMatrix& other) const;
	bool isUIdentity() const;
	
	FUMatrix& operator=(const FUMatrix& other) {
		delete [] m_data;
		m_size = other.m_size;
		m_data = new FUOver2[m_size*m_size];
		std::copy(other.m_data, other.m_data+m_size*m_size, m_data);
		
		return *this;
	}
	FUMatrix& operator=(FUMatrix&& other) {
		delete [] m_data;
		m_size = other.m_size;
		m_data = other.m_data;
		other.m_size = 0;
		other.m_data = nullptr;
		
		return *this;
	}
	
	FUOver2& operator()(unsigned int i, unsigned int j) {
		assert(i<m_size && j<m_size);
		return m_data[i*m_size+j];
	}
	
	const FUOver2& operator()(unsigned int i, unsigned int j) const {
		assert(i<m_size && j<m_size);
		return m_data[i*m_size+j];
	}
	
	friend std::istream& operator>>(std::istream& stream, std::pair<FUMatrix, unsigned int>& val);
	friend void readMatrixText(std::istream& stream, FUMatrix& val);
	friend void readEntriesText(std::istream& stream, FUMatrix& val);
};

class FUSubMatrix {
	FUMatrix& m_child;
	std::list<unsigned int> m_indexes_list;
	
public:
	typedef std::list<unsigned int>::iterator Index;
	typedef std::list<unsigned int>::const_iterator ConstIndex;
	
	FUSubMatrix(FUMatrix& child) : m_child(child) {
		for (unsigned int i=0; i<m_child.size(); i++)
			m_indexes_list.push_back(i);
	}
	
	Index firstIndex() {return m_indexes_list.begin();}
	ConstIndex firstIndex() const {return m_indexes_list.cbegin();}
	Index endIndex()  {return m_indexes_list.end();}
	ConstIndex endIndex() const {return m_indexes_list.cend();}
	
	unsigned int size() const {return m_indexes_list.size();}
	FUOver2& operator()(Index i, Index j) {return m_child(*i, *j);}
	const FUOver2& operator()(Index i, Index j) const {return m_child(*i, *j);}
	
	void conjAij (Index i, Index j);
	void conjAijU(Index i, Index j);
	
	void removeRowColumn(Index i) {m_indexes_list.erase(i);}
};

void lemma23(FUMatrix& m, unsigned int k, std::list<Arrow>& voidArrows, std::list<Arrow>& fullArrows);

std::ostream& operator<<(std::ostream& stream, const FUOver2& val);
std::istream& operator>>(std::istream& stream, FUOver2& val);
std::ostream& operator<<(std::ostream& stream, const FUMatrix& val);
std::istream& operator>>(std::istream& stream, FUMatrix& val);



class DeterministMarkov {
	size_t m_size = 0;
	unsigned int* m_data = nullptr;
	
public:
	DeterministMarkov() = default;
	DeterministMarkov(size_t size);
	DeterministMarkov(const DeterministMarkov& other);
	DeterministMarkov(DeterministMarkov&& other);
	~DeterministMarkov() {delete [] m_data;}
	
	static DeterministMarkov Identity(size_t size);
	
	size_t size() const {return m_size;}
	DeterministMarkov operator*(const DeterministMarkov& other) const;
	DeterministMarkov& operator*=(const DeterministMarkov& other);
	
	unsigned int& operator[](size_t i) {assert(i<m_size); return m_data[i];}
	unsigned int operator[](size_t i) const {assert(i<m_size); return m_data[i];}
	DeterministMarkov& operator=(const DeterministMarkov& other);
	DeterministMarkov& operator=(DeterministMarkov&& other);
};

class DeterministMarkovPower {
	std::vector<DeterministMarkov> m_per;
	
	static size_t getNumberOfPermutationForMaxPower(size_t maxPower);
	
public:
	DeterministMarkovPower() = default;
	DeterministMarkovPower(unsigned int size, size_t maxPower);
	DeterministMarkovPower(const DeterministMarkov& deterministMarkovPost, size_t maxPower);
	DeterministMarkovPower(DeterministMarkov&& deterministMarkov, size_t maxPower);
	
	unsigned int size() const {return m_per[0].size();}
	// Returne la puissance maximal pouvant etre calcule (qui peut etre plus grande que celle envoye au constructeur)
	unsigned int getMaxPower() const;

	unsigned int getValuePower(unsigned int val, size_t power) const;
	unsigned int getMaxValue(unsigned int val) const {return m_per[m_per.size()-1][val];}
	DeterministMarkov getPower(size_t power) const;
	std::pair<size_t, bool> getWeight(unsigned int val) const;
};

#endif // __MATRIX_HPP__
