#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <string>

template<class ... Types>
class Command {
public:
	virtual void run(Types ...) = 0;
	virtual std::string name() const = 0;
	virtual void clear() {delete this;}
	virtual ~Command() {}
};

#endif // __COMMAND_HPP__

