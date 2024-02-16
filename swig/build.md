# Python

## Install

1. Compile SWIP interface:
```
swig -python -cpperraswarn -o bee2py/bee2_wrap.c -outdir bee2py bee2.i 
```
2. Install package to Python:
```
python3 -m pip install .
```

## Test
```
python3 test/test_belt.py 
python3 test/test_bign.py 
```

# Go

## Install

1. Compile SWIP interface:
```
swig -go -cgo -intgosize 64 -cpperraswarn -o bee2go/bee2_wrap.c -outdir bee2go bee2.i 
```
2. Insert 
```
#cgo LDFLAGS: -L/usr/local/lib -lbee2_static
```
after start of first long comment
```
package bee2

/*
```
3. Install package to Go:
```
go install ./bee2go
```

## Test


# R

## Install

1. Compile SWIP interface:
```
swig -r -cpperraswarn -o bee2r/src/bee2_wrap.c -outdir bee2r/R bee2.i 
```
2. Install package to Python:
```
python3 -m pip install .
```

## Test
```
python3 test/test_belt.py 
python3 test/test_bign.py 
```
