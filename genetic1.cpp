#include <bits/stdc++.h>
using namespace std;


/* =========================================================
   PARAMETERS
   ========================================================= */

const int POPULATION_SIZE = 6;
const int CHROMOSOME_LENGTH = 4;
const int GENERATIONS = 20;

const double CROSSOVER_RATE = 0.80;
const double MUTATION_RATE = 0.05;


/* =========================================================
   1. GENERATE RANDOM CHROMOSOME
   ========================================================= */

string randomChromosome(mt19937 &gen)
{
    string chromosome = "";

    for (int i = 0; i < CHROMOSOME_LENGTH; i++)
    {
        chromosome += (gen() % 2) + '0';
    }

    return chromosome;
}


/* =========================================================
   2. CREATE INITIAL POPULATION
   ========================================================= */

vector<string> createPopulation(mt19937 &gen)
{
    vector<string> population;

    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        population.push_back(randomChromosome(gen));
    }

    return population;
}


/* =========================================================
   3. CONVERT BINARY TO INTEGER
   ---------------------------------------------------------
   Example:
   0111 -> 7
   1010 -> 10
   ========================================================= */

int getX(string chromosome)
{
    int x = 0;

    for (char bit : chromosome)
    {
        x = x * 2 + (bit - '0');
    }

    return x;
}


/* =========================================================
   4. FITNESS FUNCTION
   ---------------------------------------------------------
   f(x) = 15x - x^2

   CHANGE THIS FUNCTION FOR ANOTHER PROBLEM.
   ========================================================= */

int fitness(string chromosome)
{
    int x = getX(chromosome);

    return 15 * x - x * x;
}


/* =========================================================
   5. ROULETTE WHEEL SELECTION
   ---------------------------------------------------------
   Each chromosome gets an interval according to its fitness.

   Example:

   Gene    Fitness    Interval

   0001       14      [0,14)
   0010       26      [14,40)
   0011       36      [40,76)
   ...

   Generate a random point and find the interval
   containing that point.
   ========================================================= */

string rouletteSelection(vector<string> &population,
                         mt19937 &gen)
{
    int totalFitness = 0;

    for (string chromosome : population)
    {
        totalFitness += fitness(chromosome);
    }


    // index -> {start, end}
    map<int, pair<int, int>> intervals;

    int start = 0;

    for (int i = 0; i < population.size(); i++)
    {
        int end = start + fitness(population[i]);

        intervals[i] = {start, end};

        start = end;
    }


    // Random point on roulette wheel
    uniform_int_distribution<int> randomPoint(
        0, totalFitness - 1
    );

    int point = randomPoint(gen);


    // Find which interval contains the point
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
   6. SELECT PARENTS
   ========================================================= */

vector<string> selectParents(vector<string> &population,
                             mt19937 &gen)
{
    vector<string> parents;

    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        parents.push_back(
            rouletteSelection(population, gen)
        );
    }

    return parents;
}


/* =========================================================
   7. UNIFORM CROSSOVER
   ---------------------------------------------------------
   Each bit has a 50% chance of coming from either parent.

   Example:

   Parent 1 = 1010
   Parent 2 = 0111

   Children are created by exchanging individual bits.
   ========================================================= */

pair<string, string> crossover(string parent1,
                               string parent2,
                               mt19937 &gen)
{
    string child1 = parent1;
    string child2 = parent2;

    uniform_real_distribution<double> probability(0.0, 1.0);

    // Crossover does not happen
    if (probability(gen) > CROSSOVER_RATE)
    {
        return {child1, child2};
    }


    // Uniform crossover
    for (int i = 0; i < CHROMOSOME_LENGTH; i++)
    {
        if (gen() % 2 == 1)
        {
            swap(child1[i], child2[i]);
        }
    }

    return {child1, child2};
}


/* =========================================================
   8. MUTATION
   ---------------------------------------------------------
   Flip each bit with MUTATION_RATE probability.
   ========================================================= */

void mutation(string &chromosome,
              mt19937 &gen)
{
    uniform_real_distribution<double> probability(0.0, 1.0);

    for (int i = 0; i < CHROMOSOME_LENGTH; i++)
    {
        if (probability(gen) < MUTATION_RATE)
        {
            if (chromosome[i] == '0')
                chromosome[i] = '1';
            else
                chromosome[i] = '0';
        }
    }
}


/* =========================================================
   9. CREATE NEXT GENERATION
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
   10. FIND BEST CHROMOSOME
   ========================================================= */

string findBest(vector<string> &population)
{
    string best = population[0];

    for (string chromosome : population)
    {
        if (fitness(chromosome) > fitness(best))
        {
            best = chromosome;
        }
    }

    return best;
}


/* =========================================================
   11. PRINT POPULATION
   ========================================================= */

void printPopulation(vector<string> &population)
{
    cout << "\nGene\t x\tFitness\n";

    for (string chromosome : population)
    {
        cout << chromosome << "\t"
             << getX(chromosome) << "\t"
             << fitness(chromosome) << endl;
    }
}


/* =========================================================
   MAIN
   ========================================================= */

int main()
{
    mt19937 gen(42);

    // Step 1: Generate initial population
    vector<string> population =
        createPopulation(gen);


    cout << "Initial Population";
    printPopulation(population);


    // Step 2: Repeat for generations
    for (int generation = 1;
         generation <= GENERATIONS;
         generation++)
    {
        cout << "\n\n==============================";
        cout << "\nGeneration " << generation;
        cout << "\n==============================";

        printPopulation(population);


        // Find best solution
        string best = findBest(population);

        cout << "\nBest chromosome = "
             << best;

        cout << "\nBest x = "
             << getX(best);

        cout << "\nBest fitness = "
             << fitness(best);


        // Stopping criterion
        // Maximum fitness is 56
        if (fitness(best) == 56)
        {
            cout << "\n\nStopping criterion reached!";
            break;
        }


        // Generate next generation
        population =
            createNextGeneration(population, gen);
    }


    // Final result
    string best = findBest(population);

    cout << "\n\n==============================";
    cout << "\nFINAL RESULT";
    cout << "\n==============================\n";

    cout << "Best chromosome = "
         << best << endl;

    cout << "Best x = "
         << getX(best) << endl;

    cout << "Maximum fitness = "
         << fitness(best) << endl;

    return 0;
}