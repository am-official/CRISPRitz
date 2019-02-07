// TAKED FROM http://www.geeksforgeeks.org/aho-corasick-algorithm-pattern-searching/

// C++ program for implementation of Aho Corasick algorithm for string matching
using namespace std;
#include <bits/stdc++.h>
#include <string>
#include <omp.h>
#include <vector>
#include <algorithm>
#include <cmath>

#include <bitset>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Max number of states in the matching machine.
// Should be equal to the sum of the length of all keywords.
#define MAXS 10000
#define MAXC 93
#define MAXW 2560

int i;

// OUTPUT FUNCTION IS IMPLEMENTED USING out[]
// Bit i in this mask is one if the word with index i
// appears when the machine enters this state.
bitset<MAXW> out[MAXS];

// FAILURE FUNCTION IS IMPLEMENTED USING f[]
short int f[MAXS];

// GOTO FUNCTION (OR TRIE) IS IMPLEMENTED USING g[][]
short int g[MAXS][MAXC];

// Builds the string matching machine.
// arr -   array of words. The index of each keyword is important:
//         "out[state] & (1 << i)" is > 0 if we just found word[i]
//         in the text.
// Returns the number of states that the built machine has.
// States are numbered 0 up to the return value - 1, inclusive.
void buildMatchingMachine(string arr[], int k)
{

   // Initialize all values in output function as 0.
   memset(g, -1, sizeof g);
   memset(f, 0, sizeof f);

   // Initially, we just have the 0 state
   int state = 0, currState = 0, index = 0;
   string str;

   // Construct values for goto function, i.e., fill g[][]
   // This is same as building a Trie for arr[]
   for (int i = 0; i < k; ++i)
   {
      const string &word = arr[i];
      short int currState = 0;

      // Insert all characters of current word in arr[]
      for (int j = 0; j < word.size(); ++j)
      {
         index = word[j] - 33;
         if (g[currState][index] == -1)
         {
            g[currState][index] = ++state;
         }
         currState = g[currState][index];
      }
      out[currState].set(i);
   }

   ///Failure function
   queue<int> q;
   int s, fail;
   for (i = 0; i < MAXC; i++)
   {
      if (g[0][i] != -1)
      {
         f[g[0][i]] = 0; ///here, depth is 1
         q.push(g[0][i]);
      }
      else
      {
         g[0][i] = 0; ///Necessary in failure alg below, non-existing char back to state 0. To stop infinite loop at line 68.
      }
   }
   while (!q.empty())
   {
      s = q.front();
      q.pop();
      for (i = 0; i < MAXC; i++)
      {
         if (g[s][i] != -1)
         {
            q.push(g[s][i]);
            fail = f[s]; ///here is the perfect place to calculate failure of g[s][i],cuz here 'state:s' is (depth-1) state of 'state:g[s][i]'.
            while (g[fail][i] == -1)
            {
               fail = f[fail];
            }
            fail = g[fail][i];
            f[g[s][i]] = fail;
            out[g[s][i]] |= out[fail]; ///merging output of the node & it's failure node.
                                       ///Read the paper of aho-corasick,published in 1975.
         }
      }
   }
}

// This function finds all occurrences of all array words in text.
void searchWords(vector<int> &indices, string arr[], int k, string text,int pamlen,int pamlimit)
{

   int l = text.size();
   //int nThr = omp_get_max_threads();

   int j, i = 0;
   int mez_K = k / 2;
   double start, end;
   int currentState = 0; // Initialize current state
   int ch;

   int threads = omp_get_max_threads();
   int tid;
   int chunck = 1 + ((l - 1) / threads);
   int chunckfine;

   vector<int> indices_private;

   // Build machine with goto, failure and output functions
   buildMatchingMachine(arr, k);

// Traverse the text through the nuilt machine to find all occurrences of words in arr[]
//#pragma parallel omp for private(j, currentState, ch)
   #pragma omp parallel private(tid, j, currentState, ch, i, chunckfine, indices_private)
   {
      tid = omp_get_thread_num();
      chunckfine = ((tid + 1) * chunck) + ((arr[0].size()) - 1);
      //cout << "i " << (tid*chunck) << endl;
      for (i = (tid * chunck); i < MIN(chunckfine, l); i++)
      {
         // If goto is not defined, use failure function
         ch = text[i] - 33;
         while (g[currentState][ch] == -1)
            currentState = f[currentState];
         currentState = g[currentState][text[i] - 33];

         // If match not found, move to next state
         if (out[currentState] == 0)
            continue;

         // Match found, print all matching words of arr[] using output function.
         if (out[currentState].count() == 1) //check if can be present the same PAM on both strands
         {

            if ((out[currentState]._Find_first() < mez_K)) //check what pam was found, if the first half, it's a positive pam, negative otherwise
            {
               if ((i - (pamlen - 1 + 2)) >= 0) //save the pam position only if possible for a guide to attach that position(avoid out of bound)
               {
                  indices_private.push_back(i - (pamlen - 1 + 2));
               }
            }
            else
            {
               if ((i - (pamlimit - 1)) <= (l - (pamlen+2))) //same as for positive pam(out of bound problem)
               {
                  indices_private.push_back(-(i - (pamlimit - 1)));
               }
            }
         }
         else
         {
            if ((out[currentState]._Find_first() < mez_K)) //check what pam was found, if the first half, it's a positive pam, negative otherwise
            {
               if ((i - (pamlen - 1 + 2)) >= 0) //save the pam position only if possible for a guide to attach that position(avoid out of bound)
               {
                  indices_private.push_back(i - (pamlen - 1 + 2));
               }
               if ((i - (pamlimit - 1)) <= (l - (pamlen+2))) //same as for positive pam(out of bound problem)
               {
                  indices_private.push_back(-(i - (pamlimit - 1)));
               }
            }
            else
            {
               if ((i - (pamlimit - 1 + 2)) <= (l - (pamlen+2))) //same as for positive pam(out of bound problem)
               {
                  indices_private.push_back(-(i - (pamlimit - 1)));
               }
            }
         }
      }
      #pragma omp critical
      {
         //copy all threads values in a single value usable to search on genome
         indices.insert(indices.end(), indices_private.begin(), indices_private.end());
      }
   }

}