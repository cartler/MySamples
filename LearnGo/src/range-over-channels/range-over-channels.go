package main

import "fmt"

func main() {

	queue := make(chan string, 2)
	done := make(chan bool)
	queue <- "one"
	queue <- "two"
	// close(queue)

	go func() {
		for elem := range queue {
			fmt.Println(elem)
		}
		done <- true
	}()

	queue <- "three"
	queue <- "four"
	queue <- "five"

	<-done
}
