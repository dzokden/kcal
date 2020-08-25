
## Compiling 

Dependencies 
 * ncurses-devel ncurses 

gcc/Linux ```make -f makefile.gcc```

When ready  makefiles could be merged with platform specific make targets.

## Example usage 

See Kālacakra calendar software.pdf for extended usage info

```
./kcal 2020 # Summary output for a year
./kcal sfous00.txt San_Francisco,_US 2000 -122.4 
  # Creates detailed calender in sfous00.txt with location specific data
  # kcal $outfile $location,_info $year $long
```
