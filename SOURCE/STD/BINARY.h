#ifndef BINARY_H
#define BINARY_H

#define FLAG_SET(x, flag) x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)
#define IS_FLAG_SET(x, flag) ((x & (flag)) != 0)
#define IS_FLAG_UNSET(x, flag) ((x & (flag)) == 0)

#endif // BINARY_H