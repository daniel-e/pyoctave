# pyoctave

runpy is an Octave package to run Python scripts.

## Steps

### Get the sources

```
git clone git@github.com:daniel-e/pyoctave.git
```

### Compile

```
make
```

### Octave

```
octave -p path
```

### Run Python scripts

```
octave:1> v = runpy("pyexample", "counter", "10")
v =

   0
   1
   2
   3
   4
   5
   6
   7
   8
   9
```

Now, you can work with this vector in Octave, e.g. create a 5 times 2 matrix with the command `reshape`.

```
octave:2> reshape(r, 5, 2)
ans =

   0   5
   1   6
   2   7
   3   8
   4   9
```

