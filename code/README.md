### This dir concludes *<u>every piece of</u>* source code for ***AFEA***

#### *FILE STRUCTURE*

```
|-- code
    |-- AutoEncoder.py
    |-- README.md
    |-- Reinforcement_Agent.py
    |-- IOtool
    |-- MFEA
    |-- basic_algorithm
    |-- external_accelerating
    |-- internal_accelerating
```

> IOtool
>
> > To coordinate data form between different algorithms and parts wherever data of problem need to be read and result need to be written, we defined some necessary read and write functions to help accessing experiments' data easily.

> MFEA
>
> > This is a complete and nice implementation for ***Multi-Factorial Evolutionary Algorithm*** which proposed in paper [Multifactorial Evolution: Towards Evolutionary Multitasking](https://ieeexplore.ieee.org/abstract/document/7161358) by ***Gupta et al***. It is worth mentioning that some improvement and adjustment has been taken because specific problem situation faced here. For example, we borrow the  preferred mutation and crossover from paper [Permutation based MFEA](https://www.researchgate.net/profile/Yuan-Yuan-132/publication/313585479_Evolutionary_multitasking_in_permutation-based_combinatorial_optimization_problems_Realization_with_TSP_QAP_LOP_and_JSP/links/5beb8a1792851c6b27bd1021/Evolutionary-multitasking-in-permutation-based-combinatorial-optimization-problems-Realization-with-TSP-QAP-LOP-and-JSP.pdf) by ***Yuan et al***. A concrete and more detailed doc can be found in MFEA dir.

> basic_algorithm
>
> > Just simple and plain ***Evolutionary Algorithms***, including ACO, DE, GA.

> external_accelerating
>
> > When saying ***external***, it means accelerating those EAs by technology/methods beyond algorithm itself. In this dir,  we mainly consider and implement some accelerating methods proposed in recent years which are related with ***Parallel Computing*** . In other words, to parallel EAs on CPU/GPU is mainstream of research on ***external accelerating*** EAs. Each implemented methods and its corresponding paper is reduced in table below.

| GA_CPU1      | [Hierarchical Genetic Algorithms](https://link.springer.com/chapter/10.1007/3-540-45356-3_86) |
| :----------- | ------------------------------------------------------------ |
| GA_CPU2      | [Dynamic Cellular GA](https://ieeexplore.ieee.org/document/1413255) |
| GPU_Cellular | [**GPU implementation of a cellular genetic algorithm**](https://link.springer.com/article/10.1007/s10878-016-0007-y) |
| GPU_plain    | [An efficient parallel genetic algorithm solution ](https://journalofcloudcomputing.springeropen.com/articles/10.1186/s13677-020-0157-4) |

> internal_accelerating
>
> > Compared with external, internal means some improvement of algorithm itself. For GA, those improvements includes ***coding style, ways of crossover/mutation/selec***t and so on. Each implemented methods and its corresponding paper is reduced in table below.

| internal_1 |      |
| ---------- | ---- |
| internal_2 |      |

> AutoEncoder.py
>
> > Important component of ***AFEA***. Key idea behind is that: if a framework wants to solve different NP problem simultaneously, it should know the nature distribution of these problems, which means their representations show locate in a same feature space. AutoEncoder properly supports this kind of representation. To address the nature distribution(clustering feature) of those problem instances in feature space, we adopt and change naive AE to a modified version according to paper [**Auto-encoder Based Data Clustering**](https://link.springer.com/content/pdf/10.1007/978-3-642-41822-8_15.pdf) proposed by ***Song et al***. in 2013. Besides, we add a new error to this modified version to fit situations of ***AFEA***, where an DRL agent will be connected behind AE and give ***a third error function*** to  optimize/adjust/justify training result.  You should look our paper for ***AFEA*** for more detail.

> Reinforcement_Agent.py
>
> > As the overview ***in root/README.md*** said, Reinforcement learning is  introduced to learn the similarity in history relations between problem instance and corresponding solution. To testify different RL algorithms' performance and their compatibility for ***AFEA***, two conceptually different RL methods are implemented. That is, ***online Q-learning***(value based) and ***online Actor to Critic***(A3C, policy gradient based). The former implementation refers to paper [**Asynchronous Methods for Deep Reinforcement Learning**](http://proceedings.mlr.press/v48/mniha16.pdf) by Mnih et al. in 2016 while the latter refers to [Playing Atari with Deep Reinforcement Learning](https://arxiv.org/pdf/1312.5602.pdf?source=post_page---------------------------) by Mnih et al. in 2013. The work flow of RL agent can be found in our GECCO 2021 paper [An Efficient Computational Approach for Automatic Itinerary Planning on Web Servers]().

