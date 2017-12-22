#include "io.hpp" 

void readMatrixText(std::istream& stream, FUMatrix& val) {
	val.m_data = new FUOver2[val.m_size*val.m_size];
	for (unsigned int i=0; i<val.m_size; i++) {
		char c;
		do {
			c = stream.get();
		} while ((c=='\n' || c==' ') && stream.good());
		stream.unget();
		
		if (!stream.good()) goto return_error;
		for (unsigned int j=0; j<val.m_size; j++) {
			stream >> val(i, j);
			if (stream.fail()) goto return_error;
		}
	}
	
	return;
return_error:
	delete [] val.m_data;
	val.m_data = nullptr;
	stream.setstate(std::istream::failbit);
}

void readEntriesText(std::istream& stream, FUMatrix& val) {
	val.m_data = new FUOver2[val.m_size*val.m_size];
	while (1) {
		unsigned int i, j;
		stream >> i;
		if (stream.eof()) return;
		if (i >= val.m_size || !stream.good()) goto return_error;
		
		stream >> j;
		if (j >= val.m_size || !stream.good()) goto return_error;
		
		stream >> val(i, j);
		if (stream.fail()) goto return_error;
	}
	
	return;
return_error:
	delete [] val.m_data;
	val.m_data = nullptr;
	stream.setstate(std::istream::failbit);
}

std::ostream& operator<<(std::ostream& stream, const FUOver2& val) {
	stream << val.getU() << "*U+" << val.getCst();
	return stream;
}

std::istream& operator>>(std::istream& stream, FUOver2& val) {
	char c;
	do {
		c = stream.get();
	} while (c==' ' && stream.good());
	stream.unget();
	if (!stream.good()) {
		return stream;
	}
	
	char u_c = stream.get();
	bool u;
	if (!stream.good() || (u_c != '0' && u_c != '1')) {
		stream.setstate(std::istream::failbit);
		return stream;
	} else {
		u = (u_c == '1');
	}
	
	c = stream.get();
	if (stream.good() && (c==' ' || c==',')) {
		while (c==' ' && stream.good()) {
			c = stream.get();
		}
		stream.unget();
		if (!stream.good()) {
			return stream;
		}
		
		
		if (stream.get()!=',' || !stream.good()) {
			return stream;
		}
		
		do {
			c = stream.get();
		} while (c==' ' && stream.good());
		stream.unget();
		if (!stream.good()) {
			return stream;
		}
		
		c = stream.get();
	}
	else if (stream.good() && c=='*') {
		char cs[2];
		stream.read(cs, 2);
		if (!stream.good() || cs[0]!='U' || cs[1]!='+') {
			stream.setstate(std::istream::failbit);
			return stream;
		}
		
		c = stream.get();
	}
	
	bool cst;
	if (!stream.good() || (c != '0' && c != '1')) {
		stream.setstate(std::istream::failbit);
		return stream;
	} else {
		cst = (c == '1');
	}
	
	val = FUOver2(u, cst);
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const std::pair<FUMatrix, unsigned int>& val) {
	for (unsigned int i=0; i<val.first.size(); i++) {
		stream.put('[');
		for (unsigned int j=0; j<val.first.size(); j++) {
			stream << val.first(i, j);
			char arr[2];
			arr[0] = (j==val.first.size()-1) ? ']': ',';
			arr[1] = (j==val.first.size()-1) ? '\n' : ' ';
			stream.write(arr, 2);
		}
	}
	
	return stream;
}

std::istream& operator>>(std::istream& stream, std::pair<FUMatrix, unsigned int>& val) {
	val.first = FUMatrix();
	std::string s;
	std::getline(stream, s);
	size_t j;
	if (!stream.good()) goto return_error;
	
	// On eleve les espaces
	j=0;
	for (size_t i=0; i<s.length(); ++i) {
		if (s[i]!=' ')
			s[j++] = s[i];
	}
	s.resize(j);
	
	stream >> val.second;
	if (val.second > 10000 || !stream.good()) goto return_error;
	
	stream >> val.first.m_size;
	if (val.first.m_size > 10000 || !stream.good()) goto return_error;
	
	if (s == "matrix")
		readMatrixText(stream, val.first);
	else if (s == "entries")
		readEntriesText(stream, val.first);
	else
		stream.setstate(std::istream::failbit);
	
	return stream;
return_error:
	stream.setstate(std::istream::failbit);
	return stream;

}
