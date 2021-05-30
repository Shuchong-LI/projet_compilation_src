int start = 0;
int end = 10;

void main()
{
	int i, s = start, e = end;
	int sum = 0;
    do {
        i = i + 1;
		sum = sum + i;
    }
    while (sum < end);
	print("sum: ", sum, "\n");
}
