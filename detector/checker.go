package main

import (
	"fmt"
	"os"
	"os/exec"
)

func main() {
	modules := [...]string{"./check-hash", "./sys_detect"}
	var exitCode int
	var detected int = 0
	for _, module := range modules {
		cmd := exec.Command(module)
		err := cmd.Run()
		if err == nil {
			exitCode = 0
		} else {
			exitCode = 1
			detected += 1
		}
		fmt.Printf("%s exit with %d\n", module, exitCode)
	}
	if detected > 0 {
		os.Exit(1)
	}
	os.Exit(0)
}
