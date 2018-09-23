#include <stdio.h>

int main(){
	int a, x, y, u, dx;
	while(x < a)
	{
		u = u - ( 3 * x * u * dx ) - ( 3 * y * dx );
		y = y + u * dx;
		x = x + dx;
	}
	printf("Value of y is = %d\n",y);
	return(y);
}
