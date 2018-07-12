#include "dskcf_tracker_run.hpp"

int main(int argc, const char** argv)
	{

				for (double a = 0.1; a <1; a += 0.1)
		 {
		 DskcfTrackerRun main;
		 char mul_str[256];
		 sprintf(mul_str, "%.2f", a);
		 argv[2] = mul_str;
		 main.start(argc, argv);

		 }

/*		DskcfTrackerRun main;

		if (main.start(argc, argv))
			{
				return 0;
			}
		else
			{
				return -1;
			}*/
	}
