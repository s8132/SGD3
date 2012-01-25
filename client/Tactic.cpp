#include "Tactic.h"

char Tactic::fair(){
	char fair = 'f';
	return fair;
}

char Tactic::unfair(){
	char unfair = 'u';
	return unfair;
}

char Tactic::fityFity(int x){
	char result;
	if(x%2==0){
		result = Tactic::fair();
	}else{
		result = Tactic::unfair();
	}
	return result;
}

char Tactic::titForTat(char x){
	if((int)x==120){
		char result = Tactic::fair();
		return result;
	}else{
		return x;
	}
}