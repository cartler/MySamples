// SFINAE.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <type_traits> // std::is_integral

using namespace std;

// 用法1.返回值重载

// template<bool B, class T = void>
// struct enable_if {};
// 
// template<class T>
// struct enable_if<true, T> { typedef T type; };


template<typename T,
	typename enable_if<is_integral<T>::value, int>::type n = 0>
	void what_am_i(T) {
	cout << "Integral number." << endl;
}

template<typename T,
	typename enable_if<is_floating_point<T>::value, int>::type n = 0>
	void what_am_i(T) {
	cout << "Floating point number." << endl;
}

template<typename T,
	typename enable_if<is_pointer<T>::value, int>::type n = 0>
	void what_am_i(T) {
	cout << "Pointer." << endl;
}

int main()
{
	what_am_i(123);
	what_am_i(123.0);
	what_am_i("123");

	getchar();
}


// int _tmain(int argc, _TCHAR* argv[])
// {
// 	return 0;
// }

