package main

import (
	"bufio"
	"crypto/md5"
	"encoding/hex"
	"fmt"
	"io"
	"log"
	"os"
	"strings"
)

func main() {
	file, err := os.Open("hash.txt")
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	var misMatch int = 0
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		s := strings.Fields(scanner.Text())
		f, err := os.Open(s[0])
		if err != nil {
			return
		}
		defer f.Close()

		h := md5.New()
		if _, err := io.Copy(h, f); err != nil {
			log.Fatal(err)
		}
		src := []byte(h.Sum(nil))

		dst := make([]byte, hex.EncodedLen(len(src)))
		hex.Encode(dst, src)

		if string(dst) != s[1] {
			fmt.Printf("%s Hash Not Match\n", s[0])
			misMatch += 1
		}
	}
	if misMatch > 0 && misMatch < 5 {
		os.Exit(1)
	}
	os.Exit(0)
}
