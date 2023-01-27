from random import randint, getrandbits
import os


def generate_rand(require_different_digits=False):
    num_digit_a = randint(10, 20)
    num_digit_b = randint(10, 20) if require_different_digits else num_digit_a
    return randint(10 ** num_digit_a, 10 ** (num_digit_a + 1)), \
        randint(10 ** num_digit_b, 10 ** (num_digit_b + 1))


def generate_test_cases():
    flag = bool(getrandbits(1))

    # add
    print("========= add =========")
    print()

    # t01: 同位数加法
    x, y = generate_rand(require_different_digits=False)
    print(f"{x}+{y} = {x + y}")
    print()

    # t02: 不同位数加法
    x, y = generate_rand(require_different_digits=True)
    print(f"{x}+{y} = {x + y}")
    print()

    # mul
    print("========= mul =========")
    print()

    # t03: 同位数乘法
    x, y = generate_rand(require_different_digits=False)
    print(f"{x}*{y} = {x * y}")
    print()

    # t04: 不同位数乘法
    x, y = generate_rand(require_different_digits=True)
    print(f"{x}*{y} = {x * y}")
    print()

    # t05: 操作数为0的乘法
    x, _ = generate_rand(require_different_digits=False)
    if flag:
        print(f"{x}*{0} = 0")
    else:
        print(f"{0}*{x} = 0")
    print()

    # neg
    print("========= neg =========")
    print()

    # t06: 负数加法
    x, y = generate_rand(require_different_digits=False)
    if flag:
        x = -x
    else:
        y = -y
    print(f"{x}+{y} = {x + y}")
    print()

    # t07: 负数乘法
    x, y = generate_rand(require_different_digits=True)
    if flag:
        x = -x
        y = -y
    else:
        y = -y
    print(f"{x}*{y} = {x * y}")
    print()


def main():
    clear_command = "cls" if os.name == "nt" else "clear"
    while True:
        generate_test_cases()
        print("按回车键重新生成用例")
        input()
        os.system(clear_command)


if __name__ == "__main__":
    main()
