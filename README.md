# C-Parallelization-of-a-genetic-algorithm
Pentru inceput am creat o structura prin care am reusit sa transimt mai multe variabile thread-urilor pe care le-am creat.
M-am gandit ca, daca eu nu pot sa folosesc variabile globale, am sa manageriez toate variabiele in main pentru ca mai apoi
sa le transmit threadu-urilor. Deci, toate varibilele se afle in maine si thread-urile doar lucreaza pe ele. Am modificat 
functia read_input pentru a putea citi si numarul de threaduri in momentul in care rulez programul si tot in main am 
initializat current_generation, next_generation si current_generation_new care ma ajuta la merge sort. In mod normal as fi
initializat current_generation, next generation si current_generation_new de capacitatea lui objcent_count insa merge sortul 
functioneaza pe vectori care au dimensiunea egala cu o 2^n. Prin urmare, dupa ce am citit object_count, am aflat valearea de 2^n 
pe care as putea sa o folosesc si mi-am initilizat vectorii dupa valoarea aflata. Dupa acest lucru mi am initiliazizat valorile 
pe care aveam sa le trimit threadurilor si le-am creat. De aici am sa trec la explicarea thread-urilor.
In momentul in care un thread era creat, am luat din argumentele de le cream toate valorile si dupa am inceput sa paralelizez.
Am incercat sa paralelizez toate forurile dupa forumula pe care am invatat-o la laboratoare. Sortarea am schimbat-o cu merge pentru cat
mai multa scalabilitate.
Bariera am transmis-o tot prin structura pentru ca am initializat-o in main, si am utilizat-o in urmatoarele locuri. Am folosit bariera
dupa apelarea functiilor de calculare a fitness-ului pentru ca dupa aceasta apelare trebuie sa ordonez vectorul dupa aceste valori si 
doresc ca toate threadurile sa termine de calculat fitness-ul ca sa nu incep sa ordonez pe valori necalculate de fitness. Dupa acest lucru
am pus o bariera dupa crossover pentru ca doresc ca toate threadurile sa termine mutatiile de facut inainte ca current_generation sa o fac 
next_generation si sa incep sa lucrez pe current_generation. 
La crossover a trebuie sa fac o modificare deoarece for-ul mergea din 2 in 2. Astfel daca aveam mai multe threaduri, era posbil ca start-ul
sa fie numar impar si sa incep de unde nu trebuie.
Dupa ce se termina rularea foru-ului pe cate generatii am, am repetat procedeul de apelare a functiei care imi calculeaza fitness-ul , bariera,
si dupa ordonarea acestui vector.
In mare, tot ce am crezut eu ca am avut de paralelizat a fost for-urile si desigur qsort-ul pe care l-am inlocuit cu merge sort.
