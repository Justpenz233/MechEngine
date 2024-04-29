cd Source/ || exit
find . -name "*.cpp" -o -name "*.h" | grep -v '/_generated/'| grep -v '/UI/' | grep -v '/3rd/'| grep -v '/BuildTool/' | xargs wc -l