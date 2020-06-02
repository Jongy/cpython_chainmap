from chainmap import ChainMap

x = ChainMap()

try:
    x[5]
except KeyError:
    pass

a = {"a": 4}
b = {"b": 5}
x = ChainMap((a, b))
assert x["a"] == 4
assert x["b"] == 5
a["b"] = 6
b["c"] = 7
assert x["b"] == 6
assert x["c"] == 7

x = ChainMap((1, ))
try:
    x[5]
except TypeError as e:
    assert e.args[0] == "'int' object is not subscriptable"

print("All okay!")
