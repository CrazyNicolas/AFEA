### This dir includes the implementation for MFEA

MFEA ***Multi-Factorial Evolutionary Algorithm*** which proposed in paper [Multifactorial Evolution: Towards Evolutionary Multitasking](https://ieeexplore.ieee.org/abstract/document/7161358) by ***Gupta et al***. It is worth mentioning that some improvement and adjustment has been taken because specific problem situation faced here. For example, we borrow the preferred mutation and crossover from paper [Permutation based MFEA](https://www.researchgate.net/profile/Yuan-Yuan-132/publication/313585479_Evolutionary_multitasking_in_permutation-based_combinatorial_optimization_problems_Realization_with_TSP_QAP_LOP_and_JSP/links/5beb8a1792851c6b27bd1021/Evolutionary-multitasking-in-permutation-based-combinatorial-optimization-problems-Realization-with-TSP-QAP-LOP-and-JSP.pdf) by ***Yuan et al***.

#### *CODE STRUCTURE*

> ***Matrix.h*** : Maintain a matrix and provide matrix operators for convenience of calculating.
>
> > **Constructors** : Construct a matrix instance: filled with 0s by assigning thw height and width / an one dimension matrix by passing in a double type array and the length / a full matrix by passing a two dimension arrary and height and width / overrided copy function

> > **Overrided Functions** :
> > 
> > > *Operator[]* : Return a pointer pointing to the head of a row of matrix to enable direct subscript access; i.e. Matrix[i][j]
> > > 
> > > *Matrix +/-/ \* double* : Add / Subtract / Multiply a value to each position.
> > >
> > > *Matrix +/- Matrix* : Add / Subtract the values of the corresponding positions, the dimensions of two matrix should be the same.
> > > 
> > > *Matrix * Matrix* : Multiply two matrixes following the matrix multiplication rule.

> > **Other Functions**
> > 
> > > *trans* : Transform the matrix.
> > > 
> > > *cut* : Return a left-upper sub-matrix with assigned height and width.
> > > 
> > > *Rand* : Return a matrix with assigned height and width, filled with random value in [0, 1].
> > >
> > > *print* : Print matrix.

> ***Problem.h*** : A base class of problems for polymorphism which contains a pure virtual method *solve* which recrives an one-dimension solution matrix and returns the result value.
> 
> > **Ackley.h** : The Ackley problem class, implement as the MFEA paper. Maintain a random shift matrix *M*, dimension of problem *dim*, the optimal solution *opt* and the *center* and *radius* of data range.
> > 
> > **Rastrigin.h** : The Rastrigin problem class, implement as the MFEA paper. Maintain a random shift matrix *M*, dimension of problem *dim*, the optimal solution *opt* and the *center* and *radius* of data range.
> > 
> > **Sphere.h** : The Sphere problem class, implement as the MFEA paper. Maintain the dimension of problem *dim*, the optimal solution *opt* and the *center* and *radius* of data range.
> > 
> > **TSP.h** : The TSP problem class, maintains the dimension of problem(the number of nodes) *dim* and a distance matrix *map*.

> ***Individual.h*** : The class maintaining the attributes and behaviours of individuals in GA population.
>
> > **Members** : 
> > 
> > > *length* : the length of gene sequence.
> > > 
> > > *gene* : An one-dimension Matrix as the GA gene sequence.
> > >
> > > *fracorial[number of problems]* : An array maintains the cost value corresponding to each problem.
> > >
> > > *skill_factor* : The index of problem where the individual performs best over all problems.
> > > 
> > > *scalar_fitness* : 1 / *skill_factor* .

> > **Methods** :
> >
> > > *Constructor* : Assign the length of gene and number of problems.
> > > 
> > > *Init* : Initialize gene with random value in [0, 1].
> > >
> > > *Int_Init* : Initialize gene to a permutation of node indexes.
> > >
> > > *Update_Cost* : Calculate the cost of individual under each problem.

> ***Population.h*** : The class to perform GA operations.
>
> > **Members** :
> > 
> > > *dim & size* : The dimensions of individuals and the population size.
> > > 
> > > *Individual popul[population size]* : The array implementation of GA population.
> > >
> > > *int factorial_rank[population size][number of problems]* : Maintain a rank table where shows the rank of each individual for each problem.i.e. The cost of individual *i* for problem *j* is the *x*th lowest over all individuals for this problem, then *factorial_rank[i][j] = x* .
> > >
> > > *cr_rate* : The crossover rate.
> > >
> > > *off_scale* : The proportion of offspring size.
> >
> > **Methods** : 
> > 
> > > *Constructor* : Assign parameters above.
> > > 
> > > *Init* : Call initialization function of each individual.
> > > 
> > > *Updata_Factor* : Update the dactorial_cost table and individuals' skill factor and scalar fitness.
> > >
> > > *Init_Evaluate* : Update Individuals' fracorial table and call *Update_Factor* to update tables.
> > > 
> > > Crossover Function : 
> > > 
> > > > *SBX* : Implementation of Simulated Binary Crossover where u = 2.
> > > >
> > > > *PMX* : Implementation of Partial-Mapped Crossover.
> > >
> > > Mutation Function : 
> > > 
> > > > *Gaussian_Mu* : Implementation of Gaussian Mutation where u = 5.
> > > >
> > > > *Swap_Mu* : A mutation operation by randomly swap two values in gene.
> > > 
> > > *getOffspring* : Generate offsprings using crossover and mutation functions as described in the paper.
> > >
> > > *Select* : Select the best *size* individuals to make up the next generation.
> > > 
> > > *Get_Best* : Get the best individual of a problem.

> ***main.cpp*** : The entrance of MFEA. The calling command is **./MFEA number_of_problem problem1_datapath problem2_datapath ...** . For each path, *main.cpp* uses methods included in IOtool dir to read coordinate data to construct distance matrices.Then execute GA opertors on initialized population and print the best individual for each problem.