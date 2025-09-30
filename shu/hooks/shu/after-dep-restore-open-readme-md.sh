
if command -v code &> /dev/null; then
    code "README.md" &> /dev/null
elif command -v gedit &> /dev/null; then
    gedit "README.md" &> /dev/null
elif command -v xdg-open &> /dev/null; then
    xdg-open "README.md" &> /dev/null
else
    echo ""
    echo "Important! Please open 'README.md' in your preferred text editor to view the project documentation."
    echo ""
fi