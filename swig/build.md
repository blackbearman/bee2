# Python

## Install

1. Compile SWIG interface:
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

1. Compile SWIG interface:
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
go mod init by.bsu/bee2
go build by.bsu/bee2
go install by.bsu/bee2
```

## Test
```
go run ../test/test_bign.go
```

# R

## Install

1. Compile SWIG interface:
```
swig -r -cpperraswarn -module bee2r -o bee2r/src/bee2_wrap.c -outdir bee2r/R bee2.i 
```
2. Install package to R:
```
install.packages("devtools")
devtools::install()
```

## Test
```
Rscript test/test_bign.R 
```

# Swift

## Install

Swift don't need a wrapper for C code from version 4.2. 
Umbrella header and module.modulemap files should be in a separate directory.

## Build
```
cd bee2swift
swift build 
```
## Test
```
swift run 
```


# C#

## Install

1. Compile SWIG interface:
```
swig -csharp -o bee2net/bee2_wrap.c -outdir bee2net -outfile bee2cs.cs -dllimport bee2wrap -namespace bcrypto bee2.i
```
2. Compile and build wrapper library:
```
mkdir -p bee2net/runtimes/linux-x64/native
gcc bee2net/bee2_wrap.c --shared -lbee2_static -o bee2net/runtimes/linux-x64/native/bee2wrap.so
```
3. Compile C# project:
```
cd bee2net
[dotnet new classlib --force
sed -i 's|</Project>|\
  <ItemGroup>\
    <Content Include=\"runtimes\\**\">\
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>\
    </Content>\
  </ItemGroup>\
</Project>|g' bee2net.csproj]
dotnet build
```

## Test
```
cd test/test_csharp
dotnet build
dotnet run
```
