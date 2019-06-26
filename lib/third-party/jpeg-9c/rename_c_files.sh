for file in *.c; do
    mv "$file" "$(basename "$file" .c).cpp"
done
