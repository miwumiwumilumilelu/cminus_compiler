
int fibonacci(int n)
{
    int cnt;
    int firstFib;
    int secondFib;
    int fib;

    firstFib = 1;
    secondFib = 1;
    cnt = 2; /* n = 1 或 n = 2 时特判 */

    if (n == 1)
        return 1;
    else if (n == 2)
        return 1;
    else
    {
        while (cnt < n)
        {
            fib = firstFib + secondFib;
            firstFib = secondFib;
            secondFib = fib;
            cnt = cnt + 1;
        }
    }
    return fib;
}

void main(void)
{
    int n;
    scanf("%d", &n);
    printf("Fibo(n)=%d\n", fibonacci(n));
}
