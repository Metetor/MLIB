## compute z-address
### test rjust;[dim_bits is the m]:suppose value range of all data points is in [0,2^{m-1}]
# x=4
# y=3
# zaddr=['0']*6
# zaddr[0::2]=bin(x)[2:].rjust(3,'0')
# zaddr[1::2]=bin(y)[2:].rjust(3,'0')

# print(int(''.join(zaddr),2))