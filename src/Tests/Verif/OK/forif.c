void main()
{
	int a;
	for (a = 0; a < 10; a = a + 1) {
		if (a % 2 == 0) {
			print(a);
		} else {
			if (a != 9)
				print("-");
		}
	}
}