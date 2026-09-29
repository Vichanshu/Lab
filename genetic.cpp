#include <bits/stdc++.h>
using namespace std;


/* =========================================================
   PARAMETERS
   ========================================================= */

const int POPULATION_SIZE = 6;
const int CHROMOSOME_LENGTH = 4;
const int GENERATIONS = 20;

const double MUTATION_RATE = 0.10;
const double CROSSOVER_RATE = 0.80;


/* =========================================================
   1. CREATE RANDOM CHROMOSOME
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
   3. CONVERT BINARY CHROMOSOME TO INTEGER
   ========================================================= */

int getValue(string chromosome)
{
    int value = 0;

    for (char bit : chromosome)
    {
        value = value * 2 + (bit - '0');
    }

    return value;
}


/* =========================================================
   4. FITNESS FUNCTION
   ---------------------------------------------------------
   CHANGE THIS FOR YOUR PROBLEM
   ========================================================= */

double fitness(string chromosome)
{
    int x = getValue(chromosome);

    // Example:
    // f(x) = 15x - x^2

    return 15 * x - x * x;
}


/* =========================================================
   5. ROULETTE WHEEL SELECTION
   ---------------------------------------------------------
   We create intervals according to fitness contribution.

   Example:

   A fitness = 10
   B fitness = 20
   C fitness = 30
   D fitness = 40

   Intervals:

   A -> [0, 10)
   B -> [10, 30)
   C -> [30, 60)
   D -> [60, 100)

   Then generate one random number and find
   which interval contains it.
   ========================================================= */

string rouletteSelection(vector<string> &population,
                         mt19937 &gen)
{
    double totalFitness = 0;

    // Calculate total fitness
    for (string chromosome : population)
    {
        totalFitness += fitness(chromosome);
    }


    /*
       map stores:

       chromosome -> {start, end}

       Example:

       "1010" -> {0, 20}
       "0110" -> {20, 50}
       ...
    */

    map<string, pair<double, double>> intervals;

    double start = 0;

    for (string chromosome : population)
    {
        double contribution = fitness(chromosome);

        double end = start + contribution;

        intervals[chromosome] = {start, end};

        start = end;
    }


    // Generate random point on roulette wheel
    uniform_real_distribution<double> randomNumber(
        0.0, totalFitness
    );

    double randomPoint = randomNumber(gen);


    // Find which interval contains randomPoint
    for (auto &entry : intervals)
    {
        string chromosome = entry.first;

        double left = entry.second.first;
        double right = entry.second.second;

        if (randomPoint >= left &&
            randomPoint < right)
        {
            return chromosome;
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
        string parent =
            rouletteSelection(population, gen);

        parents.push_back(parent);
    }

    return parents;
}


/* =========================================================
   7. CROSSOVER
   ---------------------------------------------------------
   Uniform crossover
   ========================================================= */

pair<string, string> crossover(string parent1,
                               string parent2,
                               mt19937 &gen)
{
    string child1 = parent1;
    string child2 = parent2;

    uniform_real_distribution<double> probability(0.0, 1.0);

    // Check whether crossover happens
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
   9. CREATE NEW POPULATION
   ========================================================= */

vector<string> createNewPopulation(vector<string> &population,
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
    for (string chromosome : population)
    {
        cout << chromosome
             << "  x = " << getValue(chromosome)
             << "  fitness = " << fitness(chromosome)
             << endl;
    }
}


/* =========================================================
   MAIN
   ========================================================= */

int main()
{
    mt19937 gen(42);

    // Initial population
    vector<string> population =
        createPopulation(gen);


    cout << "Initial Population:\n";
    printPopulation(population);


    // Run GA
    for (int generation = 1;
         generation <= GENERATIONS;
         generation++)
    {
        cout << "\n============================\n";
        cout << "Generation " << generation << "\n";
        cout << "============================\n";

        printPopulation(population);


        string best = findBest(population);

        cout << "\nBest chromosome: "
             << best << endl;

        cout << "Best value: "
             << getValue(best) << endl;

        cout << "Best fitness: "
             << fitness(best) << endl;


        // Create next generation
        population =
            createNewPopulation(population, gen);
    }


    // Final result
    string best = findBest(population);

    cout << "\n============================\n";
    cout << "FINAL RESULT\n";
    cout << "============================\n";

    cout << "Best chromosome: "
         << best << endl;

    cout << "Best value: "
         << getValue(best) << endl;

    cout << "Best fitness: "
         << fitness(best) << endl;

    return 0;
}