#!/bin/sh

echo 'ls -a; echo hello && mkdir test || echo world; git status exit' | ../bin/test.out
