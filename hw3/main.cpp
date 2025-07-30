/******************************************************************************/
/* This is the program skeleton for homework 3 in CSE167x by Ravi Ramamoorthi */
/* Extends HW 2 to deal with ray casting and rendering                        */
/******************************************************************************/

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include <FreeImage.h>
#include "readfile.h"
#include "variables.h"

using namespace std; 

int main(int argc, char* argv[]) {

  if (argc < 2) {
    cerr << "Usage: raycast scenefile\n"; 
    exit(-1); 
  }

  FreeImage_Initialise();
  
  // Parse the scene file
  string scenefile = argv[1];
  cout << "Parsing scene file: " << scenefile << endl;
  
  try {
    readfile(scenefile.c_str());
  } catch (int e) {
    cerr << "Error parsing scene file: " << e << endl;
    FreeImage_DeInitialise();
    exit(-1);
  }
  
  // TODO: Implement ray casting and rendering
  cout << "Ray casting and rendering not yet implemented" << endl;
  
  FreeImage_DeInitialise();
  return 0;
} 