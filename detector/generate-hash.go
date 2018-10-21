package main

import (
	"crypto/md5"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
)

func hash(file string, fo *os.File) {
	f, err := os.Open(file)
	if err != nil {
		return
	}
	defer f.Close()

	h := md5.New()
	if _, err := io.Copy(h, f); err != nil {
		log.Fatal(err)
	}

	//fmt.Printf("%s %x\n", file, h.Sum(nil))
	fmt.Fprintf(fo, "%s	%x\n", file, h.Sum(nil))
}

func main() {
	targets := [...]string{"/bin", "/sbin"}
	fo, err := os.Create(os.Args[1])
	if err != nil {
		log.Fatal("Fail to create file", err)
	}
	defer fo.Close()
	for _, target := range targets {
		files, err := ioutil.ReadDir(target)
		if err != nil {
			//log.Fatal(err)
			continue
		}

		for _, f := range files {
			hash(target+"/"+f.Name(), fo)
		}
	}
}
