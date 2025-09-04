#include <unistd.h>
#include <stdio.h>

int ft_atoi(const char *str)
{
    int res = 0;
    while (*str)
    {
        if (*str >= '0' && *str <= '9')
            res = res * 10 + (*str - '0');
        else
            break;
        str++;
    }
    return res;
}

void print_solution(int *x, int n)
{
    for (int i = 1; i <= n; ++i)
    {
        for (int j = 1; j <= n; ++j)
        {
            if (x[j] == i)
            {
                fprintf(stdout, "%d ", j-1);
            }
        }
    }
    fprintf(stdout, "\n");
}

int is_safe(int *x, int k)
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

void solve(int *x, int k, int n)
{
    if (k > n)
    {
        print_solution(x, n);
        return;
    }
    for (int i = n; i >= 1; --i)
    {
        x[k] = i;
        if (is_safe(x, k))
            solve(x, k + 1, n);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2 || *argv[1] == '0')
        return (1);
    int n = ft_atoi(argv[1]);
    int x[n + 1];
    solve(x, 1, n);
    return 0;
}

