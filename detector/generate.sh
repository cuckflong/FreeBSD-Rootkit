#!/bin/sh

env GOOS=freebsd GOARCH=386 go build check-hash.go
env GOOS=freebsd GOARCH=386 go build generate-hash.go
env GOOS=freebsd GOARCH=386 go build checker.go

tar -cvf detector.tar detect checker check-hash sys_hook_detection.c sys_list_file.h hash.txt