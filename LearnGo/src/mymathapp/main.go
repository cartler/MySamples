package main

import (
	"errors"
	"fmt"
	"math"
	"mymath"
	"time"
)

const Pi = 3.1415926

var i int = 10000

const MaxThread = 10
const prefix = "astaxie_"

func main() {
	fmt.Printf("Hello, world.  Sqrt(2) = %v\n", mymath.Sqrt(2))

	s := "hello"
	s = "c" + s[1:] // 字符串虽不能更改，但可进行切片操作
	fmt.Printf("%s\n", s)

	m := `hello
    world`
	fmt.Print(m)
	ff := `http://www.gooble.com\boo.com`
	fmt.Print(ff)

	a := [3]int{1, 2, 3} // 声明了一个长度为3的int数组

	b := [10]int{1, 2, 3} // 声明了一个长度为10的int数组，其中前三个元素初始化为1、2、3，其它默认为0

	c := []int{4, 5, 6} // 可以省略长度而采用`...`的方式，Go会自动根据元素个数来计算长
	for i, num := range a {
		fmt.Println("index: %d, %d", i, num)
	}

	for i, num := range b {
		fmt.Println("index: %d, %d", i, num)
	}

	for i, num := range c {
		fmt.Println("index: %d, %d", i, num)
	}

	err := errors.New("emit macho dwarf: elf header corrupted")
	if err != nil {
		fmt.Println(err)
		fmt.Println("test")
	}

	i = 100
	const (
		Pi        = 3.1415926
		i         = 10000
		MaxThread = 10
		prefix    = "astaxie_"
	)

	fmt.Println(math.Sqrt(9))

	switch time.Now().Weekday() {
	case time.Saturday, time.Sunday:
		fmt.Println("it's the weekend")
	default:
		fmt.Println("it's a weekday")
	}

	defer func() {
		if x := recover(); x != nil {
			fmt.Print(x)
		}
	}()

	var array [100]int
	ii := 100
	array[ii] = 0

	myFunc()

}

func myFunc() {
	i := 0
Here: //这行的第一个词，以冒号结束作为标签
	println(i)
	i++
	goto Here //跳转到Here去
}
