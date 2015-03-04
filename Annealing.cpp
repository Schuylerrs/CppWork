/****************************************************
 * Schuyler Summers
 * Best, Good, Better
 * In that order because that is the order this program
 * gives solutions in. It gives the best soltuion
 * found by brute force, then it gives an initial state
 * then it gives the state found by simulated annealing.
 *
 * ACTIVITY LOG
 * 6/20 Read through the whole PDF, skimmed the 
 *   Wikipedia page, and searched for code examples
 *
 * 6/25 worked with group 1 (Ian, Joe, Aaron, me) to
 *   come up with a strategy to come up with
 *   something that works. We're going to make a one
 *   dimensional annealing algorithm work then extend
 *   it to work for a four dimensional one.
 *
 * 6/26 Searched others' code online for ideas
 *   Developed a rough hashing program to count collisions
 *
 * 6/27 Met with group 1, compared our codes. 
 *	 Created working code. 
 *	 - I came up with the idea to change the hash table 
 *   from an array of ints to an array of bools. The numbers
 *   were not relevant to the problem all that mattered was
 *   if the bucket had an object stored in it.
 *   - Joe got the idea to implement dynamic programming by
 *   storing the relationships between potential neighbors
 *   in an array.
 *   - Aaron came up with the idea to make the state a struct
 *   - Group came up with the idea of converting the words to
 *   their hash numbers as we read them in so that they are
 *   only converted once.
 *   
 * 6/28 Met with group 2 (Paulo, Guilherme, John Morgan, me)
 *   Created completely different code.
 *   Difference in approach State stored as an object
 *   with member functions rather than a struct
 *   Their code will be included at the end in a comment
 *   block
 * 
 * 6/28 Ran a test to compare the brute force with the annealing 
 * The brute force results on the big dictionary:
 * Number of buckets: 1048576
 * Final:
 *       x = 31
 *       y = 4
 *       z = 28
 *       w = 25
 *       Collisions 252259
 *
 * Time taken: 23679.43 seconds (6 hours)
 * 
 * The simulated annealing function got:
 * FINAL:
 *	x = 4
 *	y = 9
 *	z = 3
 *	w = 1
 *	Collisions 252578
 *
 * Time taken: 283.61 seconds
 * By comparison that's only 319 more collisions in only 4.5 minutes.
 *
 * 7/1 Made a few small changes:
 *   - Moved filename out of the struct
 *   - Allowed for temp and cooling rate to be changed
 *   to see how this effects the findings compared to
 *   the time required to find the temp
 ***************************************************/

#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <cstdlib>
#include <climits>
#include <cmath>
#include <time.h>

using namespace std;

#define randNum() rand()/(double)RAND_MAX
#define MAXSEARCH 32

//this will make it easy for me to pass info around
int pizza[32][32][32][32];

struct globalInfo
{
   int x;
   int y;
   int z;
   int w;
   int energy;
   double temperature;
   int runLimit;
   int sucLimit;
   int numberOfBuckets;
   bool * buckets;
};

vector<unsigned int> wordCodes;

void readFile (globalInfo & state, const string fileName);
int getEnergy (globalInfo & state);
unsigned int hashCode(string & word);
int hash(globalInfo & state, unsigned int h);
void setup(globalInfo & state);
int anneal(globalInfo & state);
bool oracle(float eDif, float temperature);
void bruteForce(int numberOfBuckets);

/****************************************************************
 *  Used this for a little debugging
 ***************************************************************/
void showState(globalInfo & state)
{
   cout << "X == " << state.x << endl
        << "Y == " << state.y << endl
        << "Z == " << state.z << endl
        << "W == " << state.w << endl
        << "Collisions " << state.energy << endl << endl;
}

/****************************************************************
 * Main is the driver which contains the basic program algorithm
 *  as stated in the Algorithm of the Gods pdf
 ***************************************************************/
int main(int argc, char* argv[])
{
   cout << "Initializing\n";
   for (int i = 0; i < 32; i++)
      for (int j = 0; j < 32; j++)
         for (int k = 0; k < 32; k++)
            for (int l = 0; l < 32; l++)
               pizza[i][j][k][l] = -1;
   
   srand(time(NULL));
   cout.setf(ios::showpoint);
   cout.precision(5);
   globalInfo state;
   globalInfo best;
   int numSuccess = 0;
   int newTemp = 0;
   double coolingRate = 0.9;
   string fileName;
   state.numberOfBuckets = 100; // Default number of buckets to use
   
   switch (argc)
   {
      case 5:
         coolingRate = ::atof(argv[4]);
      case 4:
         newTemp = atoi(argv[3]);
      case 3:
         state.numberOfBuckets = atoi(argv[2]);
      case 2:
         fileName = argv[1];
         break;
      default:
         cout << "No file of words provided to hash\n"
              << "Required Input:\n"
              << " - File path for dictionary\n"
              << "Optional Inputs:\n"
              << " - Number of buckets\n"
              << " - Initial temperature\n"
              << " - Cooling rate (between 1 and 0)\n";
         return 1;
         break;
   }

   cout << "Reading in file\n";
   
   readFile(state, fileName);
   
   //If the number of buckets is a power of 2 then it works better
   // So this insures that the number is a power of two
   state.numberOfBuckets = pow(2, ceil((log(state.numberOfBuckets) / log(2)))); 

   cout << "Number of buckets: " << state.numberOfBuckets << endl;
   state.buckets = new bool[state.numberOfBuckets];

   cout << "Brute Force Results:\n";
   bruteForce(state.numberOfBuckets);
   
   //This is the beginning of the annealing algorithm
   cout << "Initial\n";
   setup(state);
   setup(best);

   if (newTemp)
      state.temperature = newTemp;

   showState(state);

   for (int i = 0; i < 5; ++i)
   {
      //this just tells us how long we want to run the algorithm
      while (state.temperature > .000005)
      {
         numSuccess = anneal(state);
         
         if (numSuccess == 0)
            break;
         
         state.temperature *= coolingRate;
      }
      
      if (state.energy < best.energy)
      {
         best.x = state.x;
         best.y = state.y;
         best.z = state.z;
         best.w = state.w;
         best.energy = state.energy;
      }
   }
   
   cout << "Final\n";
   showState(best);
      
   delete [] state.buckets;
     
   return 0;
}

/*******************************************************************
 * ReadFile both read then converts the word into a code
 ******************************************************************/
void readFile (globalInfo & state, const string fileName)
{
   
   ifstream input(fileName.c_str());
   
   if (input.is_open())
   {
      string word;
      while (input >> word)
         wordCodes.push_back(hashCode(word));
        
      input.close();
   }
   else
      cout << "Unable to open " + fileName;  
}

/**************************************************************
 * setup will initialize the list and global info
 *************************************************************/
void setup(globalInfo & state)
{
   // getEnergy(state.energy);                  JAVA's numbers used as BEST
   state.x = ((int)(randNum() * 32) + 1) % 32;//9
   state.y = ((int)(randNum() * 32) + 1) % 32;//14
   state.z = ((int)(randNum() * 32) + 1) % 32;//4
   state.w = ((int)(randNum() * 32) + 1) % 32;//10
   state.energy = getEnergy(state);
   state.temperature = 200;
   state.runLimit = 100 * MAXSEARCH;
   state.sucLimit = 10 * MAXSEARCH;
}

/************************************************************
 * this is really the meat of the program.  I'm not really
 * sure what it does but I will comment what I do find out
 ***********************************************************/
int anneal(globalInfo & state)
{
   int numSuc = 0;
   float eDif = 0;
   globalInfo alteration;
   alteration.numberOfBuckets = state.numberOfBuckets;
   alteration.buckets = new bool [alteration.numberOfBuckets];
   
   for (int i = 0; i <= state.runLimit; ++i)
   {
      for (int j = 0; j < state.numberOfBuckets; ++j)
         alteration.buckets[j] = false;
      
//      showState(state);
      alteration.x = (state.x + ((int)(randNum() * 2 - 1 + MAXSEARCH + .5))) % MAXSEARCH;
      alteration.y = (state.y + ((int)(randNum() * 2 - 1 + MAXSEARCH + .5))) % MAXSEARCH;
      alteration.z = (state.z + ((int)(randNum() * 2 - 1 + MAXSEARCH + .5))) % MAXSEARCH;
      alteration.w = (state.w + ((int)(randNum() * 2 - 1 + MAXSEARCH + .5))) % MAXSEARCH;

//      assert(alteration.x && alteration.y && alteration.z && alteration.w); 
      alteration.energy = getEnergy(alteration);
      
      eDif = alteration.energy - state.energy;

      if (oracle(eDif, state.temperature))
      {
         state.x = alteration.x;
         state.y = alteration.y;
         state.z = alteration.z;
         state.w = alteration.w;
         state.energy = alteration.energy;
         numSuc++;
      }

      if (numSuc >= state.sucLimit)
         break;
   }
   delete [] alteration.buckets;
   return numSuc;
}

/***********************************************************
 * Magic hand waving, but no if the energy is lower aka eDif > 0
 * then the oracle agrees to change then if e to the eDif over temp
 * is greater than some random number otherwise it's false
 **********************************************************/
bool oracle(float eDif, float temperature)
{
   float e = 2.71;
   //we are looking for a min value so the diff should be positive
   if (eDif < 0 ||
       randNum() < exp(-eDif / temperature))
      return true;

   return false;
}

/************************************************************************
 * This is the function to convert astrings to ints Java's implementation
 ***********************************************************************/
unsigned int hashCode(string & word)
{
   unsigned int h = 0;
   
   for (int i = 0; i < word.length(); i++)
      h = 31 * h + word.at(i);
     
   return h ;  
}

/**********************************************************************
 * getEnergy is the function that gets the energy levels
 *********************************************************************/
int getEnergy (globalInfo & state)
{
   if (pizza[state.x][state.y][state.z][state.w] != -1)
      return pizza[state.x][state.y][state.z][state.w];
          
   int energy = 0;
   
   for (int i = 0; i < wordCodes.size(); ++i)
   {
      int h = hash(state, wordCodes.at(i));
      energy += state.buckets[h];
      state.buckets[h] = true;
   }
   
   pizza[state.x][state.y][state.z][state.w] = energy;
   return energy;
}

/***********************************************************************
 * Hash is the way we check the our energy levels
 **********************************************************************/
int hash(globalInfo & state, unsigned int h)
{   
   h += ~(h << state.x);
   h ^= (h >> state.y);   
   h += (h << state.z);   
   h ^= (h >> state.w);
   
   return (h & (state.numberOfBuckets - 1));
}

/*************************************************************************
 * Brute force finds the lowest value and fills in our entire lookup table
 ************************************************************************/
void bruteForce(int numBuckets)
{
   globalInfo best;
   globalInfo alteration;

   best.numberOfBuckets = numBuckets;
   alteration.numberOfBuckets = numBuckets;
   alteration.buckets = new bool[alteration.numberOfBuckets];
   
   for (int i = 0; i < numBuckets; i++)
       alteration.buckets[i] = false;
  
   best.energy = 100000000;
   
   for (int w = 0; w < 31; ++w)
      for (int z = 0; z < 31; ++z)
         for (int y = 0; y < 31; ++y)
            for (int x = 0; x < 31; ++x)
            {
               alteration.x = x;
               alteration.y = y;
               alteration.z = z;
               alteration.w = w;
               
               alteration.energy = getEnergy(alteration);
               
               for (int i = 0; i < alteration.numberOfBuckets; i++)
                  alteration.buckets[i] = false;
               
               if (alteration.energy < best.energy)
               {
                  best.x = alteration.x;
                  best.y = alteration.y;
                  best.z = alteration.z;
                  best.w = alteration.w;
                  best.energy = alteration.energy;
               }
            }
   
   cout << "Best case\n";
   showState(best);
   
   delete [] alteration.buckets;
}

/******************************************************************************
* I used the first code because I was more involved in it's creation and it runs
* faster
* Please compile with:   g++ SA.cpp -O3  OR g++ SA.cpp -O3 -DSHOW
* Collaborators:
*    - Guilherme Bentim
*    - John Morgan
*    - Paulo Fagundes
*#include <cmath>
*#include <cstdlib>
*#include <cstring>
*#include <ctime>
*#include <fstream>
*#include <iostream>
*#include <locale>
*#include <vector>
*using namespace std;


class State
{
   public:
      State()                           : p1(0), p2(0), p3(0), p4(0) { }
      State(int a, int b, int c, int d) : p1(a), p2(b), p3(c), p4(d) { }
      State randomize();
      int random131();

      int p1;
      int p2;
      int p3;
      int p4;

      friend ostream& operator << (ostream& out, const State& state); 
};

class SA
{
   public:
      SA(char* filename);
      State run();
      int energy(const State& state); // E()
      ~SA();
   
   private:
      vector <int> hashCodes;
      int tableSize; // size of the collisionTable
      int* collisionTable;
   
      void readfile(char* filename);
      double probability(double e, int enew, double T); // P()
      int hashCode(const string& text);
      int hash(const string& text, const State& state);
      int hash(unsigned int h, const State& state);
      int indexFor(int h, int length);
      void resetCollisionTable();
      int getTableSize();
      int nextPowerOf2(int number);
      int random01();
};

int main(int argc, char** argv)
{
   char filename[256] = "/home/neffr/words/bigdictionary.txt";
   
   if (argc > 1)
      strcpy(filename, argv[1]);

   cout << "Please wait ..." << endl;   

   SA sa(filename);

   State best = sa.run(); // run Simulated Annealing to find the best state

   // display the results
   cout.imbue(locale("")); // make it show commas in the number of collisions
   cout << "BEST STATE:   " <<  best
        <<  " - " << sa.energy(best) << " collisions" << endl;

   State state(9, 14, 4, 10); // 158,739
   cout << "\"JAVA\" STATE: " <<  state
        << " - " << sa.energy(state) << " collisions" << endl;

   return 0;
}

inline State State::randomize()
{
   return State(random131(), random131(), random131(), random131());
}


inline int State::random131()
{
   return rand() % 31 + 1;
}


ostream& operator << (ostream& out, const State& state)
{
   out << "{ " << state.p1 << ", " << state.p2 << ", "
               << state.p3 << ", " << state.p4 << " }" ;
   return out;
}

SA::SA(char* filename)
{
   srand(time(NULL)); // Seed the random number generator with the time.

   readfile(filename);

   tableSize = nextPowerOf2(hashCodes.size());

   collisionTable = new int[tableSize];
}


inline SA::~SA()
{
   delete [] collisionTable;
}


void SA::readfile(char* filename)
{
   ifstream fin(filename);
   if (fin.fail())
   {
      cout << "Error while opening " << filename << endl;
      exit(1);
   }

   string word;

   while (getline(fin, word))           // compute the hashCode for each 
      hashCodes.push_back(hashCode(word)); // word only once

   fin.close();
}


State SA::run()
{
   State s; // start a random State
   int e = energy(s); // compute the energy for the given state

   State sbest = s;
   unsigned int ebest = e;

   double temp = 100;
   double coolingRate = 1 - 0.003;

   while (temp > 1)
   {
#ifdef SHOW
      cerr << "temperature: " << temp << endl;
#endif
      State snew = s.randomize();// the 'neighbor' will be another random state
      unsigned int enew = energy(snew); // compute the new energy

      if (probability(e, enew, temp) > random01()) // should we transition?
      {
#ifdef SHOW
         cerr << "new state, new energy" << endl;
#endif
         s = snew;
         e = enew;
      }

      if (enew < ebest) // found a better state
      {
#ifdef SHOW
         cerr << "**************found better state, better energy: "
              << snew << " - " << enew << endl;
#endif
         sbest = snew;
         ebest = enew;
      }
      temp *= coolingRate;
   }

   return sbest;
}


inline double SA::probability(double e, int enew, double T)
{
   return ((enew < e) ? 1 : exp ((e - enew) / T));
}


int SA::energy(const State& state)
{
   int numHashCodes = hashCodes.size();
   int collisions = 0;

   resetCollisionTable();

   // fill the table with the hash indices
   for (int i = 0; i < numHashCodes; i++)
      collisionTable[indexFor(hash(hashCodes[i], state), tableSize)]++;

   // count num of collisions
   for (int i = 0; i < tableSize; i++)
      if (collisionTable[i])
         collisions += collisionTable[i] - 1;
   
   return collisions;
}


int SA::hashCode(const string& text)
{
   int h = 0;
   int n = text.size();

   for (int i = 0; i < n; i++)
      h = 31 * h + text[i];

   return h;
}


int SA::hash(const string& text, const State& state)
{
   unsigned int h = hashCode(text);

   h += ~(h << state.p1);
   h ^=  (h >> state.p2);
   h +=  (h << state.p3);
   h ^=  (h >> state.p4);

   return h;
}


int SA::hash(unsigned int h, const State& state)
{
   h += ~(h << state.p1);
   h ^=  (h >> state.p2);
   h +=  (h << state.p3);
   h ^=  (h >> state.p4);

   return h;
}

inline int SA::indexFor(int h, int length)
{
   return h & (length - 1);
}


void SA::resetCollisionTable()
{
   for (int i = 0; i < tableSize; i++)
      collisionTable[i] = 0;
}


inline int SA::getTableSize()
{
   return tableSize;
}


inline int SA::nextPowerOf2(int number)
{
   return pow(2, ceil(log2(number)));
}


inline int SA::random01()
{
   return rand() % 2;
}
******************************************************************************/
