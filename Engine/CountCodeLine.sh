cd Source/
find . -name "*.cpp" -o -name "*.h" | grep -v '/_generated/'| grep -v '/UI/' | xargs wc -l