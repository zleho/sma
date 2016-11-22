set key right box
set output 'aweighting.png'
plot [20:20000] (12200.0**2 * x**4) / ((x**2 + 20.6**2) * sqrt((x**2+107.7**2)*(x**2 + 737.9**2)) * (x**2 + 12200.0**2)) title 'A(f)'

