// cppTemplateTrick.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>

using namespace std;

template<typename T> struct integral_trait
{
	enum { is_integral = false, };
};
template<> struct integral_trait<char>
{
	enum { is_integral = true, };
};
template<> struct integral_trait<unsigned char>
{
	enum { is_integral = true, };
};
template<> struct integral_trait<int>
{
	enum { is_integral = true, };
};
template<> struct integral_trait<unsigned int>
{
	enum { is_integral = true, };
};

template<typename T, typename Trait = integral_trait<T>>
bool is_integral_number(T) {
	return Trait::is_integral;
}

////////////////////////////////////////////////////////////
// My classes
struct big_integer {
	big_integer(const string&) { }
};

template<> struct integral_trait<big_integer>
{
	enum { is_integral = true, };
};


int main()
{
	cout << is_integral_number(123) << endl; // 1
	cout << is_integral_number(123.) << endl; // 0
	cout << is_integral_number(big_integer("123")) << endl; // 1
	cout << is_integral_number("123") << endl; // 0

	getchar();
}


// int _tmain(int argc, _TCHAR* argv[])
// {
// 	return 0;
// }

