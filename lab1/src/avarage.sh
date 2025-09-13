total_sum=0
count=0

for num in "$@"; do
  total_sum=$((total_sum + $num))
  count=$((count + 1))
done

average=$((total_sum / $count))

echo "Количество аргументов: $count"
echo "Среднее арифметическое: $average"