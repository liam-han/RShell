#!/bin/sh

echo '$ ls -a; echo hello && mkdir test || echo world; git status; #hello world; #bye world exit' | ../bin/test.out


