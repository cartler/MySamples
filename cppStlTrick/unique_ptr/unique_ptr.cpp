// unique_ptr.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory>
#include <iostream>

#include <iostream>
#include <memory>

struct Foo
{
	int inner = 111;
	Foo()      { std::cout << "Foo::Foo\n"; }
	Foo(int i) :inner(i)	{ std::cout << "Foo:Foo(int i)\n"; };
	~Foo()     { std::cout << "Foo::~Foo\n"; }
	void bar() { std::cout << "Foo::bar\n"; std::cout << inner << std::endl; }
};

void f(const Foo &)
{
	std::cout << "f(const Foo&)\n";
}

int main()
{
// 	std::unique_ptr<Foo> p1(new Foo);  // p1 owns Foo
// 	//std::unique_ptr<Foo> p1(nullptr);  // p1 owns Foo
// 	delete p1.get();
// 	//if (p1) p1->bar();
// 	p1->bar();

	auto p1 = std::make_unique<Foo>(222);
	if (p1)
		p1->bar();

	auto p2 = std::move(p1);
	if (p2)
	{
		p2->bar();
	}

	getchar();

	p1->bar();

	//Foo *pFoo = nullptr;
// 	Foo *pFoo = new Foo;
// 	pFoo->bar();

// 	{
// 		std::unique_ptr<Foo> p2(std::move(p1));  // now p2 owns Foo
// 		f(*p2);
// 
// 		p1 = std::move(p2);  // ownership returns to p1
// 		std::cout << "destroying p2...\n";
// 	}
// 
// 	if (p1) p1->bar();

	// Foo instance is destroyed when p1 goes out of scope
	getchar();
}


// int _tmain(int argc, _TCHAR* argv[])
// {
// 	MyClass *pOriMyClass = new MyClass();
// 	delete pOriMyClass;
// 	//std::unique_ptr<MyClass> pMyClass(pOriMyClass);
// 	std::unique_ptr<MyClass> pMyClass(nullptr);
// 
// 	pMyClass->foo();
// 
// 	getchar();
// 	return 0;
// }

