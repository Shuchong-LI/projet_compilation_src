int var1 = 10;
void main()
{
	int var2, var3 = 3;
	var1 = 12;
	var2 = var1;
	print("plus");
	var2 = 1 + ( 2 + ( var1 + (5 + 3)));
	//var3 = var1 + 2;
	print(var1, " ", var2);
}