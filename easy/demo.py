import corpus

c = corpus.Corpus( "Caltech101" )
print c.name

def detect( n ):
    a, b = 0, 1
    while b < n:
        print b,
        a, b = b, a+b
        
