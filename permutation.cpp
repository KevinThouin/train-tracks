#include <cassert>
#include <algorithm>
#include <numeric>

#include "permutation.hpp"

static unsigned int matToPermutationIndex(unsigned int n, unsigned int k, unsigned int i) {
	unsigned int ret;
	
	if (i<k)
		ret = i;
	else if (i<n)
		ret = k+n-i-1;
	else if (i<n+k)
		ret = n+n+k-i-1;
	else
		ret = i;
	
	return ret;
}

Permutation::Permutation(const Permutation& other) : m_size(other.m_size) {
	m_map = new unsigned int[m_size];
	std::copy_n(other.m_map, m_size, m_map);
}

Permutation Permutation::Identity(unsigned int size) {
	Permutation ret(size);
	std::iota(ret.m_map, ret.m_map+ret.m_size, 0);
	
	return ret;
}

Permutation Permutation::operator*(const Permutation& other) const {
	assert(m_size==other.m_size);
	
	Permutation ret(m_size);
	for (unsigned int i=0; i<m_size; i++) {
		ret.m_map[i] = m_map[other[i]];
	}
	
	return ret;
}

Permutation& Permutation::preMult(const Permutation& other) {
	assert(m_size==other.m_size);
	
	for (unsigned int i=0; i<m_size; i++) {
		m_map[i] = other[m_map[i]];
	}
	
	return *this;
}

Permutation& Permutation::operator*=(std::pair<unsigned int, unsigned int> p) {
	assert(p.first < m_size);
	assert(p.second < m_size);
	
	std::swap(m_map[p.first], m_map[p.second]);
	
	return *this;
}

bool Permutation::isIdentity() const {
	for (unsigned int i=0; i<m_size; ++i) {
		if (m_map[i]!=i) return false; 
	}
	return true;
}

Pairing::Pairing(const FUMatrix& mat, unsigned int k) : m_per(Permutation::Identity(mat.size())) {
	assert(k<=mat.size()/2);
	
	for (unsigned int j=1; j<mat.size(); j++) {
		for (unsigned int i=0; i<j; i++) {
			if (mat(i, j).getCst()) {
				const unsigned int a = matToPermutationIndex(mat.size()/2, k, i);
				const unsigned int b = matToPermutationIndex(mat.size()/2, k, j);
				m_per*=std::make_pair(a, b);
			}
		}
	}
}

void Pairing::swapPoints(std::pair<unsigned int, unsigned int> p) {
	m_per *= p;
	m_per *= std::make_pair(m_per[p.first], m_per[p.second]);
}

BiPermutation::BiPermutation(const Permutation& per, bool isInverse) : m_per(per), m_inv_per(m_per.size()) {
	for (unsigned int i=0; i<m_per.size(); ++i) {
		m_inv_per.m_map[m_per[i]] = i;
	}
	
	if (isInverse) std::swap(m_per, m_inv_per);
}

BiPermutation::BiPermutation(Permutation&& per, bool isInverse) : m_per(std::move(per)), m_inv_per(m_per.size()) {
	for (unsigned int i=0; i<m_per.size(); ++i) {
		m_inv_per.m_map[m_per[i]] = i;
	}
	
	if (isInverse) std::swap(m_per, m_inv_per);
}

void BiPermutation::preAdd (std::pair<unsigned int, unsigned int> p) {
	m_per     *= p;
	m_inv_per *= std::make_pair(m_per[p.first], m_per[p.second]);
}

void BiPermutation::postAdd(std::pair<unsigned int, unsigned int> p) {
	m_per     *= std::make_pair(m_inv_per[p.first], m_inv_per[p.second]);
	m_inv_per *= p;
}
