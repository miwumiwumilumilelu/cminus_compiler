/*the maximum of three numbers*/
int max(int x, int y, int z)
{
	int biggest;
	biggest = x;
	if(y>biggest)  /*comment example*/
		biggest = y;
	if(z>biggest) biggest = z;

	return biggest;
}

void main(void)
{
	int x;
	int y;
	int z;
	int biggest;

	x = intput();
	y = input();
	z = intput();

	biggest = max(x, y, z);
	output(biggest);
}

