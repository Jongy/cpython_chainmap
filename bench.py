from collections import ChainMap
from chainmap import ChainMap as cChainMap
import timeit

a = {"a": 5}
b = {"b": 6}
c = {"c": 7}

m1 = ChainMap(a, b, c)
m2 = cChainMap(m1.maps)

print("ChainMap access first", timeit.timeit("m['a']", globals={"m": m1}))
print("C ChainMap access first", timeit.timeit("m['a']", globals={"m": m2}))

print("ChainMap access deepest", timeit.timeit("m['c']", globals={"m": m1}))
print("C ChainMap access deepest", timeit.timeit("m['c']", globals={"m": m2}))
