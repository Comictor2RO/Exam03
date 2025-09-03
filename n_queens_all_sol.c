#include <unistd.h>
#include <stdio.h>

#define N 8

int n = N;
int x[N + 1];
int solution_count = 0;

void ft_putchar(char c)
{
    write(1, &c, 1);
}

void print_board()
{
    for (int i = 1; i <= n; ++i)
    {
        for (int j = 1; j <= n; ++j)
        {
            if (x[j] == i)
                ft_putchar('Q');
            else
                ft_putchar('.');
        }
        ft_putchar('\n');
    }
    ft_putchar('\n');
}

int is_safe(int k)
{
    for (int i = 1; i < k; ++i)
    {
        if (x[i] == x[k] || 
            x[k] - x[i] == k - i || 
            x[i] - x[k] == k - i)
            return 0;
    }
    return 1;
}

void solve(int k)
{
    if (k > n)
    {
        solution_count++;
        print_board();
        return;
    }

    for (int i = 1; i <= n; ++i)
    {
        x[k] = i;
        if (is_safe(k))
            solve(k + 1);
    }
}

int main(void)
{
    solve(1);
    printf("Total solutions: %d\n", solution_count);
    return 0;
}

