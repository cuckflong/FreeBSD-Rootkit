#!/bin/sh

env GOOS=freebsd GOARCH=386 go build diskinfo.go
env GOOS=freebsd GOARCH=386 go build check-hash.go
env GOOS=freebsd GOARCH=386 go build generate-hash.go
env GOOS=freebsd GOARCH=386 go build checker.go
