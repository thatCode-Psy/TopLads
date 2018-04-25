import math


index=3
blocksize=1
assoc=2

cache_blockoffsetbits = int((math.log((blocksize * 4))/ math.log(2)));

#2  * (2^3 * 32 * 32 + 33 - 2 - 1)
cache_size = assoc * ( 2**index ) * (32 * blocksize + 33 - index - (math.log(blocksize)/math.log(2)))
print(cache_size)
print(cache_blockoffsetbits)
print(math.log(cache_size/blocksize/assoc)/math.log(2))

index=3
blocksize=1
assoc=2

cache_blockoffsetbits = int((math.log((blocksize * 4))/ math.log(2)));

#2  * (2^3 * 32 * 32 + 33 - 2 - 1)
cache_size = assoc * ( 2**index ) * (32 * blocksize + 33 - index - (math.log(blocksize)/math.log(2)))
print(cache_size)
print(cache_blockoffsetbits)
print(math.log(cache_size/blocksize/assoc)/math.log(2))