#include <bits/stdc++.h>
using namespace std;


/* =========================================================
   PARAMETERS
   ========================================================= */

const int POPULATION_SIZE = 6;
const int GENERATIONS = 20;

const double CROSSOVER_RATE = 0.80;
const double MUTATION_RATE = 0.05;

const int KNAPSACK_CAPACITY = 15;


/* =========================================================
   ITEM INFORMATION
   ========================================================= */

int weight[4] = {7, 2, 1, 9};
int value[4]  = {500, 400, 700, 200};


/* =========================================================
   1. GENERATE RANDOM GENE
   ========================================================= */

string randomGene(mt19937 &gen)
{
    string gene = "";

    for (int i = 0; i < 4; i++)
    {
        gene += (gen() % 2) + '0';
    }

    return gene;
}


/* =========================================================
   2. CREATE INITIAL POPULATION
   ========================================================= */

vector<string> createPopulation(mt19937 &gen)
{
    vector<string> population;

    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        population.push_back(randomGene(gen));
    }

    return population;
}


/* =========================================================
   3. CALCULATE TOTAL WEIGHT
   ========================================================= */

int totalWeight(string gene)
{
    int total = 0;

    for (int i = 0; i < 4; i++)
    {
        if (gene[i] == '1')
        {
            total += weight[i];
        }
    }

    return total;
}


/* =========================================================
   4. CALCULATE TOTAL VALUE
   ========================================================= */

int totalValue(string gene)
{
    int total = 0;

    for (int i = 0; i < 4; i++)
    {
        if (gene[i] == '1')
        {
            total += value[i];
        }
    }

    return total;
}


/* =========================================================
   5. FITNESS FUNCTION
   ---------------------------------------------------------
   If weight > 15, fitness = 0.
   Otherwise fitness = total value.
   ========================================================= */

int fitness(string gene)
{
    int wt = totalWeight(gene);

    if (wt > KNAPSACK_CAPACITY)
        return 0;

    return totalValue(gene);
}


/* =========================================================
   6. ROULETTE WHEEL SELECTION
   ---------------------------------------------------------
   We create an interval for every population member.

   Example:

   Gene       Fitness       Interval
   ----------------------------------
   1010         1200        [0,1200)
   0110         1100        [1200,2300)
   ...

   A random number is generated between
   0 and total fitness.

   The interval containing that number
   determines the selected gene.
   ========================================================= */

string rouletteSelection(vector<string> &population,
                         mt19937 &gen)
{
    int totalFitness = 0;

    // Find total fitness
    for (string gene : population)
    {
        totalFitness += fitness(gene);
    }

    // Map:
    // population index -> {start, end}
    map<int, pair<int, int>> intervals;

    int start = 0;

    for (int i = 0; i < population.size(); i++)
    {
        int end = start + fitness(population[i]);

        intervals[i] = {start, end};

        start = end;
    }

    // Random point on roulette wheel
    uniform_int_distribution<int> randomNumber(
        0, totalFitness - 1
    );

    int point = randomNumber(gen);

    // Find interval
    for (auto entry : intervals)
    {
        int index = entry.first;

        int left = entry.second.first;
        int right = entry.second.second;

        if (point >= left && point < right)
        {
            return population[index];
        }
    }

    return population.back();
}


/* =========================================================
   7. SELECT PARENTS
   ========================================================= */

vector<string> selectParents(vector<string> &population,
                             mt19937 &gen)
{
    vector<string> parents;

    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        string parent =
            rouletteSelection(population, gen);

        parents.push_back(parent);
    }

    return parents;
}


/* =========================================================
   8. SINGLE POINT CROSSOVER
   ========================================================= */

pair<string, string> crossover(string parent1,
                               string parent2,
                               mt19937 &gen)
{
    string child1 = parent1;
    string child2 = parent2;

    uniform_real_distribution<double> probability(0.0, 1.0);

    // No crossover
    if (probability(gen) > CROSSOVER_RATE)
    {
        return {child1, child2};
    }

    // Choose crossover point
    // For 4 bits, point can be 1, 2 or 3
    uniform_int_distribution<int> pointDistribution(1, 3);

    int point = pointDistribution(gen);

    // Exchange bits after crossover point
    for (int i = point; i < 4; i++)
    {
        swap(child1[i], child2[i]);
    }

    return {child1, child2};
}


/* =========================================================
   9. MUTATION
   ---------------------------------------------------------
   Each bit has a small probability of mutation.
   ========================================================= */

void mutation(string &gene,
              mt19937 &gen)
{
    uniform_real_distribution<double> probability(0.0, 1.0);

    for (int i = 0; i < 4; i++)
    {
        if (probability(gen) < MUTATION_RATE)
        {
            if (gene[i] == '0')
                gene[i] = '1';
            else
                gene[i] = '0';
        }
    }
}


/* =========================================================
   10. CREATE NEXT GENERATION
   ========================================================= */

vector<string> createNextGeneration(
    vector<string> &population,
    mt19937 &gen)
{
    // Selection
    vector<string> parents =
        selectParents(population, gen);

    vector<string> newPopulation;

    // Crossover + Mutation
    for (int i = 0; i < POPULATION_SIZE; i += 2)
    {
        string parent1 = parents[i];

        string parent2 =
            parents[(i + 1) % POPULATION_SIZE];

        // Crossover
        pair<string, string> children =
            crossover(parent1, parent2, gen);

        string child1 = children.first;
        string child2 = children.second;

        // Mutation
        mutation(child1, gen);
        mutation(child2, gen);

        newPopulation.push_back(child1);
        newPopulation.push_back(child2);
    }

    newPopulation.resize(POPULATION_SIZE);

    return newPopulation;
}


/* =========================================================
   11. FIND BEST GENE
   ========================================================= */

string findBest(vector<string> &population)
{
    string best = population[0];

    for (string gene : population)
    {
        if (fitness(gene) > fitness(best))
        {
            best = gene;
        }
    }

    return best;
}


/* =========================================================
   12. PRINT POPULATION
   ========================================================= */

void printPopulation(vector<string> &population)
{
    cout << "\nGene\tWeight\tValue\tFitness\n";

    for (string gene : population)
    {
        cout << gene << "\t"
             << totalWeight(gene) << "\t"
             << totalValue(gene) << "\t"
             << fitness(gene) << endl;
    }
}


/* =========================================================
   MAIN
   ========================================================= */

int main()
{
    mt19937 gen(42);

    // Step 1: Initial population
    vector<string> population =
        createPopulation(gen);

    cout << "Initial Population";
    printPopulation(population);


    // Step 2 onwards: GA
    for (int generation = 1;
         generation <= GENERATIONS;
         generation++)
    {
        cout << "\n\n==============================";
        cout << "\nGeneration " << generation;
        cout << "\n==============================";

        printPopulation(population);

        string best = findBest(population);

        cout << "\nBest Gene = " << best;
        cout << "\nBest Weight = "
             << totalWeight(best);
        cout << "\nBest Value = "
             << totalValue(best);
        cout << "\nBest Fitness = "
             << fitness(best);


        // Stop if we have found the optimal solution
        if (fitness(best) == 1600)
        {
            break;
        }


        // Create next generation
        population =
            createNextGeneration(population, gen);
    }


    // Final answer
    string best = findBest(population);

    cout << "\n\n==============================";
    cout << "\nFINAL RESULT";
    cout << "\n==============================\n";

    cout << "Gene   = " << best << endl;
    cout << "Weight = " << totalWeight(best) << " kg" << endl;
    cout << "Value  = Rs. " << totalValue(best) << endl;
    cout << "Fitness = " << fitness(best) << endl;

    return 0;
}