#include <cassert>
#include <cmath>
#include <string>
#include <iostream>
#include <limits>
#include <algorithm>
#include <numeric>

#include "matrix.hpp"
#include "util.h"

FUMatrix FUMatrix::operator*(const FUMatrix& other) const {
	assert(m_size == other.m_size);
	
	FUMatrix ret(m_size);
	
	for (unsigned int i=0; i<m_size; i++) {
		for (unsigned int j=0; j<m_size; j++) {
			FUOver2& elem = ret(i, j);
			for (unsigned int k=0; k<m_size; k++)
				elem += (*this)(i, k) * other(k, j);
		}
	}
	
	return ret;
}

bool FUMatrix::isUIdentity() const {
	for (unsigned int i=0; i<m_size; i++) {
		for (unsigned int j=0; j<m_size; j++) {
			const FUOver2& elem = (*this)(i, j);
			if (i==j && !(elem.getU() && !elem.getCst()))
				return false;
			else if (i!=j && !(!elem.getU() && ! elem.getCst()))
				return false;
		}
	}
	
	return true;
}

void FUSubMatrix::conjAij (Index i, Index j) {
	assert(i!=j);
	
	for (Index k(firstIndex()); k!=endIndex(); k++)
		(*this)(k, j) += (*this)(k, i);
	
	for (Index k(firstIndex()); k!=endIndex(); k++)
		(*this)(i, k) += (*this)(j, k);
}

void FUSubMatrix::conjAijU(Index i, Index j) {
	for (Index k(firstIndex()); k!=endIndex(); k++)
		(*this)(k, j) += (*this)(k, i) * FUOver2(true, false);
	
	for (Index k(firstIndex()); k!=endIndex(); k++)
		(*this)(i, k) += (*this)(j, k) * FUOver2(true, false);
}

static void insertArrow(unsigned int k, unsigned int n, std::list<Arrow>& voidArrows, std::list<Arrow>& fullArrows,
		std::list<Arrow>::iterator& voidPos, std::list<Arrow>::iterator& fullPos, unsigned int from, unsigned int to)
{
	assert(from<2*n && to<2*n);
	
	if (from<k && to<k) {
		fullPos = fullArrows.insert(fullPos, Arrow(from, to));
		fullPos++;
	} else if (from>=k && from<n && to>=k && to<n) {
		voidPos = voidArrows.insert(voidPos, Arrow(n-1-from, n-1-to));
	} else if (from>=n && from<n+k && to>=n && to<n+k) {
		fullPos = fullArrows.insert(fullPos, Arrow(n+k-1-from, n+k-1-to));
	} else if (from>=n+k && to>=n+k){
		voidPos = voidArrows.insert(voidPos, Arrow(from-n-k, to-n-k));
		voidPos++;
	}
}

static void doLemma23(FUSubMatrix& m, unsigned int km, unsigned int nm, std::list<Arrow>& voidArrows, std::list<Arrow>& fullArrows,
		std::list<Arrow>::iterator& voidPos, std::list<Arrow>::iterator& fullPos) {
	FUSubMatrix::Index i, j;
	
	if (m.size()==0)
		return;
	
	for (FUSubMatrix::Index offset(m.firstIndex()); offset!=m.endIndex(); offset++) {
		i = m.firstIndex();
		for (j = offset; j!=m.endIndex(); i++, j++) {
			
			if (m(i, j).getCst()) {
				// On tue les constantes sur la ligne
				FUSubMatrix::Index k(j);
				while (++k!=m.endIndex()) {
					if (m(i, k).getCst()) {
						std::cout << "1(" << *j << ", " << *k << ")" << std::endl;
						m.conjAij(j, k);
						insertArrow(km, nm, voidArrows, fullArrows, voidPos, fullPos, *j, *k);
					}
				}
				
				// On tue les U sur la ligne
				for (FUSubMatrix::Index k(m.firstIndex()); k!=m.endIndex(); k++) {
					if (m(i, k).getU()) {
						std::cout << "U(" << *j << ", " << *k << ")" << std::endl;
						m.conjAijU(j, k);
					}
				}
				
				// On tue les constantes sur la colonne
				for (FUSubMatrix::Index k(m.firstIndex()); k!=i; k++) {
					if (m(k, j).getCst()) {
						std::cout << "1(" << *k << ", " << *i << ")" << std::endl;
						m.conjAij(k, i);
						insertArrow(km, nm, voidArrows, fullArrows, voidPos, fullPos, *k, *i);
					}
				}
				
				// On tue les u sur la colonne
				for (FUSubMatrix::Index k(m.firstIndex()); k!=m.endIndex(); k++) {
					if (m(k, j).getU()) {
						std::cout << "U(" << *k << ", " << *i << ")" << std::endl;
						m.conjAijU(k, i);
					}
				}
				
				goto found_one;
			}
		}
	}
	
	// We should exit loops without the goto
	assert(false);
	UNREACHABLE();
	
found_one:
	m.removeRowColumn(i);
	m.removeRowColumn(j);
	doLemma23(m, km, nm, voidArrows, fullArrows, voidPos, fullPos);
}

void lemma23(FUMatrix& m, unsigned int k, std::list<Arrow>& voidArrows, std::list<Arrow>& fullArrows) {
	if (k > m.size())
		throw std::string("Decomposition par blocs invalide");
	
	if (m.size()%2)
		throw std::string("La matrice n'est pas de la bonne taille");
	
	if (!(m*m).isUIdentity())
		throw std::string("La matrice au carree n'est pas U*I");
	
	for (unsigned int i=0; i<m.size(); i++) {
		unsigned int bound;
		if      (i<k)              bound = k;
		else if (i<m.size()/2)     bound = m.size()/2;
		else if (i<m.size()/2 + k) bound = m.size()/2 + k;
		else                       bound = m.size();
		
		for (unsigned int j=0; j<bound; j++) {
			if (m(i, j).getCst())
				throw std::string("La matrice n'est pas strictement triangulaire superieure mod U");
		}
	}
	
	FUSubMatrix sub(m);
	auto it0 = voidArrows.end(); auto it1 = fullArrows.end();
	doLemma23(sub, k, m.size()/2, voidArrows, fullArrows, it0, it1);
}







DeterministMarkov::DeterministMarkov(size_t size) : m_size(size) {
	if (m_size>0) {
		m_data = new unsigned int[m_size];
		std::fill_n(m_data, m_size, 0);
	}
}

DeterministMarkov::DeterministMarkov(const DeterministMarkov& other) : m_size(other.m_size) {
	m_data = new unsigned int[m_size];
	std::copy_n(other.m_data, m_size, m_data);
}

DeterministMarkov::DeterministMarkov(DeterministMarkov&& other) : m_size(other.m_size), m_data(other.m_data) {
	other.m_size = 0;
	other.m_data = nullptr;
}

DeterministMarkov DeterministMarkov::Identity(size_t size) {
	DeterministMarkov ret(size);
	std::iota(ret.m_data, ret.m_data+ret.m_size, 0);
	return ret;
}

DeterministMarkov DeterministMarkov::operator*(const DeterministMarkov& other) const {
	assert(m_size==other.m_size);
	DeterministMarkov ret(m_size);
	for (size_t i=0; i<m_size; ++i) {
		ret[i] = other[m_data[i]];
	}
	
	return ret;
}

DeterministMarkov& DeterministMarkov::operator*=(const DeterministMarkov& other) {
	assert(m_size==other.m_size);
	for (size_t i=0; i<m_size; ++i) {
		m_data[i] = other[m_data[i]];
	}
	
	return *this;
}

DeterministMarkov& DeterministMarkov::operator=(const DeterministMarkov& other) {
	if (m_size!=other.m_size) {
		m_size = other.m_size;
		delete [] m_data;
		m_data = new unsigned int[m_size];
	}
	std::copy_n(other.m_data, m_size, m_data);
	
	return *this;
}

DeterministMarkov& DeterministMarkov::operator=(DeterministMarkov&& other) {
	delete [] m_data;
	m_size = other.m_size;
	m_data = other.m_data;
	other.m_size = 0;
	other.m_data = nullptr;
	
	return *this;
}

size_t DeterministMarkovPower::getNumberOfPermutationForMaxPower(size_t maxPower) {
	assert(maxPower>0);
	return (maxPower==1) ? 1 : static_cast<size_t>(msb(maxPower-1)+2);
}

DeterministMarkovPower::DeterministMarkovPower(unsigned int size, size_t maxPower) : m_per(getNumberOfPermutationForMaxPower(maxPower), DeterministMarkov(size)) {}

DeterministMarkovPower::DeterministMarkovPower(const DeterministMarkov& deterministMarkov, size_t maxPower) {
	size_t nIter = getNumberOfPermutationForMaxPower(maxPower);
	m_per.reserve(nIter);
	m_per.push_back(deterministMarkov);
	for (size_t i=1; i<nIter; i++) {
		m_per.push_back(m_per[i-1]*m_per[i-1]);
	}
}

DeterministMarkovPower::DeterministMarkovPower(DeterministMarkov&& deterministMarkov, size_t maxPower) {
	size_t nIter = getNumberOfPermutationForMaxPower(maxPower);
	m_per.reserve(nIter);
	m_per.push_back(std::move(deterministMarkov));
	for (size_t i=1; i<nIter; i++) {
		m_per.push_back(m_per[i-1]*m_per[i-1]);
	}
}

unsigned int DeterministMarkovPower::getMaxPower() const {
	return (1 << (m_per.size()-1));
}

unsigned int DeterministMarkovPower::getValuePower(unsigned int val, size_t power) const {
	size_t i=0;
	while (power!=0) {
		if (power & 1)
			val = m_per[i][val];
		
		++i;
		power >>= 1;
	}
	
	return val;
}

DeterministMarkov DeterministMarkovPower::getPower(size_t power) const {
	DeterministMarkov ret(DeterministMarkov::Identity(m_per.size()));
	
	size_t i=0;
	while (power!=0) {
		if (power & 1)
			ret*=m_per[i];
		
		++i;
		power >>= 1;
	}
	
	return ret;
}

std::pair<size_t, bool> DeterministMarkovPower::getWeight(unsigned int val) const {
	size_t power = 0;
	bool isFirst = false;
	
	assert(m_per.size()>1);
	assert(m_per[0][0]==0 && m_per[0][1]==1);
	
	if (getMaxValue(val)<2) {
		isFirst = (getMaxValue(val)==0);
		
		size_t i = m_per.size()-1;
		do {
			--i;
			if (m_per[i][val]>=2) {
				val = m_per[i][val];
				power += (1 << i);
			}
		} while (i!=0);
		++power;
	}
	
	return std::make_pair(power, isFirst);
}

