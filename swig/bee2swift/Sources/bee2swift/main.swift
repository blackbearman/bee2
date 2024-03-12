import bee2

print("Hello, world!")
print(beltHash_keep())

var buf = memAlloc(128)

var params = bign_params()
var err = bignParamsStd(&params, "1.2.112.0.2.0.34.101.45.3.1")
print(params.l)
