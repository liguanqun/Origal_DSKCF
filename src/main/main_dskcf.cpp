#include "dskcf_tracker_run.hpp"

#include <iostream>
#include <ctype.h>
#include <string>
#include <fstream>
#include <stdio.h>
#include<iomanip>
using namespace std;

int main(int argc, const char** argv)
	{

		/*				for (double a = 0.1; a <1; a += 0.1)
		 {
		 DskcfTrackerRun main;
		 char mul_str[256];
		 sprintf(mul_str, "%.2f", a);
		 argv[2] = mul_str;
		 main.start(argc, argv);

		 }*/

		std::string benchmark = argv[1];
		DIR *dp;
		struct dirent *dirp;
		std::vector<std::string> seq_name;
		if ((dp = opendir(benchmark.c_str())) == NULL)
			{
				perror("opendir error");
				exit(1);
			}
		while ((dirp = readdir(dp)) != NULL)
			{

				if ((strcmp(dirp->d_name, ".") == 0) || (strcmp(dirp->d_name, "..") == 0))
					continue;

				char dirname[100];
				benchmark.copy(dirname, benchmark.size(), 0);
				dirname[benchmark.size()] = '\0';
				std::string seq = strcat(dirname, dirp->d_name);
				seq_name.push_back(seq);
				std::cout << seq << std::endl;

			}

		for (std::vector<std::string>::iterator a = seq_name.begin(); a != seq_name.end(); a++)
			{
				DskcfTrackerRun main;

				//argv[1] = *a;
				std::cout << "current sequence is " << *a << std::endl;
				double mul = std::atof(argv[2]);
				if (main.start(argc, *a, mul))
					{
						return 0;

					}
				else
					{
						//continue;
						//return -1;
					}
			}
	}

/*		DskcfTrackerRun main;

 if (main.start(argc, argv))
 {
 return 0;
 }
 else
 {
 return -1;
 }
 }*/
