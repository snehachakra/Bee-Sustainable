#ifndef TRIVIA_H
#define TRIVIA_H

#include <vector>
#include <string>
#include <random>

struct TriviaQuestion {
    std::string question;
    std::vector<std::string> answers;
    int correct_index;
    int points_reward;
    std::vector<std::string> explanations;
};

class TriviaManager {
private:
    std::vector<TriviaQuestion> bee_questions;
    std::vector<TriviaQuestion> available_questions;
    std::default_random_engine rng;

public:
    TriviaManager();
    void InitializeQuestions();
    TriviaQuestion GetRandomQuestion();
    bool CheckAnswer(const TriviaQuestion& question, int selected_index);
    std::string GetExplanation(const TriviaQuestion& question, int selected_index);
};

extern TriviaManager gTriviaManager;

#endif // TRIVIA_H