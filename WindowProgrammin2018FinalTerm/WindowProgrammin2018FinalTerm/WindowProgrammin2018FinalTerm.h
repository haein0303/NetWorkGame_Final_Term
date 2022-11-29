#pragma once

#include "resource.h"

int calcNetId(int my_num, int calc_num) {
	
	switch (my_num) {
	case 0:
		return calc_num;
		break;
	case 1:
		if (calc_num == 0) {
			return 1;
		}
		else {
			return 2;
		}
	case 2:
		return calc_num + 1;
	}
}