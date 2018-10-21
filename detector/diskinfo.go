package main

import (
	"fmt"
	"math/rand"
	"syscall"
	"time"
)

type DiskStatus struct {
	All  uint64 `json:"all"`
	Used uint64 `json:"used"`
	Free uint64 `json:"free"`
}

// disk usage of path/disk
func DiskUsage(path string) (disk DiskStatus) {
	fs := syscall.Statfs_t{}
	err := syscall.Statfs(path, &fs)
	if err != nil {
		return
	}
	disk.All = fs.Blocks * uint64(fs.Bsize)
	disk.Free = fs.Bfree * uint64(fs.Bsize)
	disk.Used = disk.All - disk.Free
	return
}

const (
	B  = 1
	KB = 1024 * B
	MB = 1024 * KB
	GB = 1024 * MB
)

var letterRunes = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")

func RandStringRunes(n int) string {
	b := make([]rune, n)
	for i := range b {
		b[i] = letterRunes[rand.Intn(len(letterRunes))]
	}
	return string(b)
}

func main() {
	rand.Seed(time.Now().UnixNano())
	disk := DiskUsage("/")
	buffer := make([]byte, 10)
	fmt.Printf("All: %.2f B\n", float64(disk.All)/float64(B))
	fmt.Printf("Used: %.2f B\n", float64(disk.Used)/float64(B))
	fmt.Printf("Free: %.2f B\n", float64(disk.Free)/float64(B))
	//spray := RandStringRunes(20000)
	syscall.Read(0, buffer)
	fmt.Printf("All: %.2f B\n", float64(disk.All)/float64(B))
	fmt.Printf("Used: %.2f B\n", float64(disk.Used)/float64(B))
	fmt.Printf("Free: %.2f B\n", float64(disk.Free)/float64(B))
}
