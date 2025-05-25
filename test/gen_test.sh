#! /bin/bash

path=build/test_data

if [ ! -d $path ]; then
    mkdir -p $path;
fi

for i in {1..100}; do
  head -c 1K /dev/urandom | tr -dc 'a-z ' > "$path/file_$i.txt"
  if (( RANDOM % 10 == 0 )); then  # 10% of files with pattern
    echo "TEST_PATTERN" >> "$path/file_$i.txt"
  fi
done