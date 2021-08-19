#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>

void set_random_seed();
int randn(int n);


struct mm_code_maker{


    void init(int i_length, int i_num){
        length = i_length;
        num = i_num;
    }


    void generate_sequence(){
        for(int i = 0; i < length; i++){
            sequence.push_back(randn(num));
        }
    }

    void give_feedback(const std::vector<int>& attempt, int& black_hits, int& white_hits){

      black_hits = 0;

      for(int i = 0; i < length; i++){
        if(attempt[i] == sequence[i]){
          black_hits++;
        }
      }
      white_hits = getHits(attempt) - black_hits;
    }

    int getHits(std::vector<int> attempt){
      std::vector<int> sequenceNums (num, 0);
      std::vector<int> attemptNums = sequenceNums;
      for(int i = 0; i < num; i++){
        for(int j = 0; j < length; j++){
          if(attempt[j] == i){
            attemptNums[i]++;
          }
          if(sequence[j] == i){
            sequenceNums[i]++;
          }
        }
      }
      int totalHits = 0;
      for(int i = 0; i < num; i++){
        totalHits = totalHits + getMinimum(attemptNums[i],sequenceNums[i]);
      }
      return totalHits;
    }

    int getMinimum(int a, int b){
      if(b > a){
        return a;
      }
      return b;
    }


    std::vector<int> sequence;


    int length;
    int num;

};


struct mm_solver{

    void init(int i_length, int i_num){
        length = i_length;
        num = i_num;
        attempts = 0;
        depth = 0;
        jumps = 0;
        if((length == 11 && num == 15) || (length == 12 && num > 8) || (length == 13 && num > 7) || (length == 14 && num > 5) || (length == 15 && num > 5)){
          split = true;
        }
        else{
          split = false;
        }
        if(split){
          found0BH = false;
          onlyBH = true;
          limit = 15000;
          checkpoint = 0;
        }
        else{
          onlyBH = false;
          getLimit();
          for(int i = 0; i < length; i++){
            reference.push_back(0);
          }
          checkpointGuess = reference;
        }
    }

    void create_attempt(std::vector<int>& attempt){
      attempt.clear();
      if(split){
        createPartial(attempt);
      }
      else{
        if(attempts == 0){
          for(int i = 0; i < length; i++){
            attempt.push_back(randn(num));
          }
        }
        else{
          attempt = nextGuess;
        }
      }
      attempts++;
    }


    void learn(std::vector<int>& attempt, int black_hits, int white_hits){
      if(black_hits != length){
        if(split){
          learnPartial(attempt,black_hits,white_hits);
        }
        else{
          attemptLog.push_back(attempt);
          blackPegs.push_back(black_hits);
          whitePegs.push_back(white_hits);
          getValidGuess();
        }
      }
    }

    void getValidGuess(){

      bool foundGuess = false;
      while(!foundGuess){
        foundGuess = generateGuess();
        if(jumps > limit){
          depth++;
        }
      }
      jumps = 0;
      nextGuess = checkpointGuess;
      depth = 0;
    }

    bool generateGuess(){
      if(split){
        guessLength = tmpLength;
      }
      else{
        guessLength = length;
      }
      int minimum_index = guessLength - 1;
      bool potentialGuess = true;
      for(int i = attemptLog.size() - 1; i >= depth; i--){
        bool failedCurrentCheck = false;
        int blackTmp = 0;
        int whiteTmp = 0;
        hitsAttempt = reference;
        hitsGuess = reference;
        for(int subLength = 1; subLength <= guessLength; subLength++){
          bool mode = onlyBH;
          onlyBH = true;
          if(bhCheck(subLength,blackTmp,i,minimum_index)){
          onlyBH = mode;
          if(!onlyBH){
            for(int a = 0; a < subLength; a++){
              if(hitsAttempt[a] == 0){
                for(int g = 0; g < subLength; g++){
                  if(hitsGuess[g] == 0 && checkpointGuess[g] == attemptLog[i][a]){
                    whiteTmp++;
                    hitsAttempt[a]--;
                    hitsGuess[g]--;
                  }
                  if(hitsAttempt[a] != 0){
                    g = subLength;
                  }
                }
              }
            }
          }
        }
        onlyBH = mode;
          if(potentialGuess){
            potentialGuess = validGuess(blackTmp,blackPegs[i],whiteTmp,whitePegs[i],subLength);
          }
          failedCurrentCheck = !validGuess(blackTmp,blackPegs[i],whiteTmp,whitePegs[i],subLength);
          if(failedCurrentCheck){
            getMinimumIndex(minimum_index,subLength);
          }
          if(minimum_index <= subLength - 1){ // if no more information can be obtained from this attempt, we exit the subLength loop and move on to the next one
            subLength = guessLength;
          }
        }
      }
      if(potentialGuess){
        return true;
      }
      jump(minimum_index);
      return false;
    }

    bool bhCheck(int subLength, int& blackTmp, int i, int index){
      if(attemptLog[i][subLength - 1] == checkpointGuess[subLength - 1]){
        blackTmp++;
        if(!validGuess(blackTmp,blackPegs[i],0,0,subLength) || index <= subLength -1){
          return false;
        }
        hitsAttempt[subLength - 1]++;
        hitsGuess[subLength - 1]++;
      }
      return true;
    }

    bool validGuess(int guessBlackHits, int attemptBlackHits, int guessWhiteHits, int attemptWhiteHits, int subLength){
      int lengthfromEnd = guessLength - subLength;
      if(guessBlackHits > attemptBlackHits || guessBlackHits + lengthfromEnd < attemptBlackHits || ((guessWhiteHits > attemptWhiteHits || guessWhiteHits + 2*lengthfromEnd < attemptWhiteHits) && !onlyBH)){
        return false;
      }
      return true;
    }

    void getMinimumIndex(int& index, int subLength){
      if(subLength - 1 < index){
        index = subLength - 1;
      }
    }

    void jump(int index){
      for(int i = index; i >= 0; i--){
        if(checkpointGuess[i] < num -1){
          checkpointGuess[i]++;
          i = 0;
        }
        else{
          checkpointGuess[i] = 0;
        }
      }
      for(int i = guessLength - 1; i > index; i--){
        checkpointGuess[i] = 0;
      }
      jumps++;
    }

    void createPartial(std::vector<int>& attempt){
      if(!found0BH){
        for(int i = 0; i < length; i++){
          attempt.push_back(randn(num));
        }
      }
      else{
        attempt = previousAttempt;
        for(int i = checkpoint; i < tmpLength + checkpoint; i++){
          attempt[i] = nextGuess[i - checkpoint];
        }
      }
    }

    void learnPartial(std::vector<int>& attempt, int black_hits, int white_hits){
      if(!found0BH && black_hits == 0){
        found0BH = true;
        tmpLength = getTmpLength(length);
        for(int i = 0; i < tmpLength; i++){
          reference.push_back(0);
        }
        checkpointGuess = reference;
      }
      if(found0BH){
        if(black_hits == checkpoint + tmpLength){
          reference.clear();
          checkpointGuess.clear();
          attemptLog.clear();
          blackPegs.clear();
          whitePegs.clear();
          checkpoint = black_hits;
          tmpLength = getTmpLength(length - checkpoint);
          for(int i = 0; i < tmpLength; i++){
            reference.push_back(0);
          }
          checkpointGuess = reference;
          if(checkpoint + tmpLength == length){
            onlyBH = false;
            limit = 8000;
          }
        }
        std::vector<int> partialAttempt;
        for(int i = checkpoint; i < checkpoint + tmpLength; i++){
          partialAttempt.push_back(attempt[i]);
        }
        attemptLog.push_back(partialAttempt);
        blackPegs.push_back(black_hits - checkpoint);
        if(!onlyBH){
          whitePegs.push_back(white_hits);
        }
        else{
          whitePegs.push_back(0);
        }
        getValidGuess();
        previousAttempt = attempt;
      }
    }

    int getTmpLength(int effectiveLength){
      if(effectiveLength % 8 == 0){
        return 8;
      }
      return effectiveLength % 8;
    }

    void getLimit(){ // function getLimit sets the limit variable to a value that allows the code breaker to solve mastermind within 10s
      if(length < 9){ // these values were found through experimenting
        limit = 8000;
      }
      else if(length < 11){
        limit = 1500;
      }
      else if(length == 11 && num < 15){
        limit = 1000;
      }
      else if(length == 12 && num < 9){
        limit = 1000;
      }
      else if(length == 13 && num < 7){
        limit = 1000;
      }
      else if(length == 14 && num < 6){
        limit = 2000;
      }
      else if(length == 15 && num < 5){
        limit = 2500;
      }
      else if(length == 13 && num == 7){
        limit = 300;
      }
      else if(length == 15 && num == 5){
        limit = 400;
      }
      else{
        limit = 1000; // for instances above 15x15 (it would be advantageous to split for better time efficiency but this can also be used)
      }

    }

    int length;
    int num;

    int attempts, depth, jumps, limit, checkpoint, tmpLength, guessLength;
    bool onlyBH,split,found0BH;
    std::vector<int> blackPegs, whitePegs, checkpointGuess, nextGuess, hitsAttempt, hitsGuess, reference, previousAttempt;
    std::vector<std::vector<int> > attemptLog;

};

int main(){
    int t = 0;
    int s = 0;
    int maxa = 0;
    int tm = 0;
    int mina = 5000;
    set_random_seed();
    int length, num;
    std::cout << "enter length of sequence and number of possible values:" << std::endl;
    std::cin >> length >> num;
    for(int j = 0; j < 100; j++){
      int ti = std::time(0);
      mm_solver solver;
      solver.init(length, num);
      mm_code_maker maker;
      maker.init(length, num);
      maker.generate_sequence();
      int black_hits=0, white_hits=0;
      int attempts_limit = 5000;
      int attempts = 0;

      while((black_hits < length) && (attempts < attempts_limit)){
          std::vector<int> attempt;
          solver.create_attempt(attempt);
          maker.give_feedback(attempt, black_hits, white_hits);
          std::cout << "attempt: " << std::endl;
          for(int i = 0; i < attempt.size(); i++){
              std::cout << attempt[i] << " ";
          }
          std::cout << std::endl;
          std::cout << "black pegs: " << black_hits << " " << " white pegs: " << white_hits << std::endl;

          solver.learn(attempt, black_hits, white_hits);

          attempts++;
      }

      if(black_hits == length){
          std::cout << "the solver has found the sequence in " << attempts << " attempts" << std::endl;
      }
      else{
          std::cout << "after " << attempts << " attempts still no solution" << std::endl;
      }

      std::cout << "the sequence generated by the code maker was:" << std::endl;
      for(int i = 0; i < maker.sequence.size(); i++){
          std::cout << maker.sequence[i] << " ";
      }
      std::cout << std::endl;
      int tf = std::time(0);
      t = tf - ti + t;
      s = s + attempts;
      if(tf - ti > tm){
        tm = tf - ti;
      }
      if(tf - ti > 9){
        std::cout << "time was greater than 9 seconds: " << tf - ti << std::endl;
        return 0;
      }
      if(attempts > maxa){
        maxa = attempts;
      }
      if(attempts < mina){
        mina = attempts;
      }
    }
    std::cout << "time average: " << t/100.0 << std::endl;
    std::cout << "maximum time: " << tm << std::endl;
    std::cout << "attempt average: " << s/100.0 << std::endl;
    std::cout << "maximum attempts: " << maxa << std::endl;
    std::cout << "minimum attempts: " << mina << std::endl;
    return 0;
}



void set_random_seed(){
    std::srand(std::time(0));
}

int randn(int n){
    return std::rand() % n;
}
