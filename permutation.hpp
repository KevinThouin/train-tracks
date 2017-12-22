#ifndef __PERMUTATION_HPP__
#define __PERMUTATION_HPP__

#include <cassert>
#include <vector>
#include <utility>
#include <algorithm>

#include "c_cpp_bridge.h"
#include "matrix.hpp"

class BiPermutation;

class Permutation {
private:
	unsigned int  m_size;
	unsigned int* m_map;
	
	Permutation(unsigned int size) : m_size(size) {m_map = new unsigned int[m_size];}
	
public:
	static Permutation Identity(unsigned int size);
	
	Permutation(const Permutation& other);
	Permutation(Permutation&& other) : m_size(other.m_size), m_map(other.m_map) {other.m_map = nullptr; other.m_size = 0;}
	~Permutation() {delete [] m_map;}
	
	const unsigned int& operator[](unsigned int val) const {assert(val<m_size); return m_map[val];}
	Permutation  operator*(const Permutation& other) const;
	Permutation& preMult(const Permutation& other);
	Permutation& operator*=(std::pair<unsigned int, unsigned int> p);
	unsigned int size() const {return m_size;}
	bool isIdentity() const;
	
	template<class COMPARE>
	void sort(COMPARE compare);
	
	template<class COMPARE>
	bool is_sorted(COMPARE compare);
	
	Permutation& operator=(const Permutation& other) {
		if (m_size != other.m_size) {
			delete [] m_map;
			m_size = other.m_size;
			m_map = new unsigned int[m_size];
		}
		std::copy(other.m_map, other.m_map+m_size, m_map);
		
		return *this;
	}
	
	Permutation& operator=(Permutation&& other) {
		delete [] m_map;
		m_size = other.m_size;
		m_map = other.m_map;
		other.m_map = nullptr;
		other.m_size = 0;
		
		return *this;
	}
	
	friend BiPermutation;
};

class Pairing {
private:
	Permutation m_per;
	
public:
	Pairing(unsigned int size) : m_per(Permutation::Identity(size)) {}
	Pairing(const FUMatrix& mat, unsigned int k);
	
	unsigned int operator[](unsigned int val) const {return m_per[val];}
	void swapPoints(std::pair<unsigned int, unsigned int>);
	unsigned int size() const {return m_per.size();}
};

class BiPermutation {
private:
	Permutation m_per;
	Permutation m_inv_per;
	
	BiPermutation(Permutation per, Permutation inv_per) : m_per(per), m_inv_per(inv_per) {assert(m_per.size() == m_inv_per.size());}
	
public:
	BiPermutation(unsigned int size) : m_per(Permutation::Identity(size)), m_inv_per(Permutation::Identity(size)) {}
	BiPermutation(const Permutation& per, bool isInverse=false);
	BiPermutation(Permutation&& per, bool isInverse=false);
	
	unsigned int size() const {assert(m_per.size()==m_inv_per.size()); return m_per.size();}
	const unsigned int& post(unsigned int val) const {return m_per[val];}
	const unsigned int& pre (unsigned int val) const {return m_inv_per[val];}
	
	BiPermutation operator+(const BiPermutation& other) const {return BiPermutation(other.m_per*m_per, m_inv_per*other.m_inv_per);}
	void preAdd (std::pair<unsigned int, unsigned int> p);
	void postAdd(std::pair<unsigned int, unsigned int> p);
	void inverse() {std::swap(m_per, m_inv_per);}
	
	const Permutation& getPermutation() const {return m_per;}
	const Permutation& getInversePermutation() const {return m_inv_per;}
};

template<class COMPARE>
void Permutation::sort(COMPARE compare) {
	std::sort(m_map, m_map+m_size, compare);
}

template<class COMPARE>
bool Permutation::is_sorted(COMPARE compare) {
	return std::is_sorted(m_map, m_map+m_size, compare);
}

#endif // __PERMUTATION_HPP__
