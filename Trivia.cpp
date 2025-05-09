#include "Trivia.h"
#include <random>
#include <chrono>
#include <algorithm>

TriviaManager gTriviaManager;

TriviaManager::TriviaManager() {
    // Seed the random number generator
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    rng.seed(seed);
    InitializeQuestions();
}

void TriviaManager::InitializeQuestions() {
    bee_questions = {
        // Question 1
        {
            "Who is the boss of the hive and lays all the eggs?",
            {"Worker Bee", "Queen Bee", "Drone Bee", "Baby Bee"},
            1,  // Queen Bee is correct (index 1)
            1000,
            {
                "Almost! Worker bees have many important jobs, like guarding the hive, collecting nectar, and pollinating flowers. But we don't lay eggs. Come back and try this flower again!",
                "Correct! The Queen is in charge and lays all the eggs.",
                "Almost! Drone bees mate with the Queen, but they can't lay eggs. Come back and try this flower again!",
                "Not quite! Baby bees can not lay eggs. Come back and try this flower again!"
            }
        },

        // Question 2
        {
            "What are some jobs that worker bees do every day?",
            {"Laying eggs and fighting bugs", "Sleeping and buzzing", "Collecting nectar, making honey, and caring for babies", "Flying far away from the hive"},
            2,  // Collecting nectar is correct (index 2)
            1000,
            {
                "Almost! Worker bees do guard the hive, but we can't lay eggs. Come back and try this flower again!",
                "Not quite! Worker bees have many important jobs to do to keep the hive healthy and safe. Come back and try this flower again!",
                "Correct! These are just a few of the many important jobs worker bees have to keep their hive safe and healthy.",
                "Not quite! On average, worker bees actually stay within a mile of the hive during the day. Come back and try this flower again!"
            }
        },

        // Question 3
        {
            "What is the main job of drone bees?",
            {"Collect nectar", "Guard the hive", "Help the queen make more bees", "Make honey"},
            2,  // Help the queen is correct (index 2)
            1500,
            {
                "Not quite! Drone bees do not collect nectar. They only have one job, which is to help the queen with something. What is it? Come back and try this flower again!",
                "Not quite! Drone bees do not guard the hive. They only have one job, which is to help the queen with something. What is it? Come back and try this flower again!",
                "Correct! Drone bees only have one job, which is to mate with the queen.",
                "Not quite! Drone bees do not make honey. They only have one job, which is to help the queen with something. What is it? Come back and try this flower again!"
            }
        },

        // Question 4
        {
            "Why do bees visit flowers? What do they collect?",
            {"Rain and dirt", "Nectar and pollen", "Honey and leaves", "Seeds and stems"},
            1,  // Nectar and pollen is correct (index 1)
            2000,
            {
                "Not quite! Think – what can you get from inside a flower? Come back and try this flower again!",
                "Correct! Bees collect nectar and pollen from flowers. The nectar becomes honey, and pollen helps plants reproduce.",
                "Not quite! Think – what can you get from inside a flower? Come back and try this flower again!",
                "Almost! Think – what can you get from inside a flower? Come back and try this flower again!"
            }
        },

        // Question 5
        {
            "What sticks to bees when they visit flowers, helping plants grow?",
            {"Water", "Leaves", "Pollen", "Honey"},
            2,  // Pollen is correct (index 2)
            1500,
            {
                "Not quite! Hint – it's inside the flower. Come back and try this flower again!",
                "Not quite! Hint – it's inside the flower. Come back and try this flower again!",
                "Correct! Pollen sticks to bees, and we spread it to other flowers which helps them grow more seeds and fruits.",
                "Not quite! Bees make honey, but what sticks to us when we visit flowers? Come back and try this flower again!"
            }
        },

        // Question 6
        {
            "What happens when bees spread pollen from flower to flower?",
            {"The flowers wilt", "The flowers get sticky", "Fruits and veggies grow", "Bees get tired"},
            2,  // Fruits and veggies grow is correct (index 2)
            1200,
            {
                "Not quite! Pollination helps flowers. Come back and try this flower again!",
                "Not quite! Come back and try this flower again!",
                "Correct! Pollen sticks to bees, and we spread it to other flowers which helps them grow more seeds and fruits.",
                "Not quite! Although we do get tired from a long day's work, when we spread pollen, it helps the flowers! Come back and try this flower again!"
            }
        },

        // Question 7
        {
            "Name two foods we eat that need bees to grow?",
            {"Apples and almonds", "Pizza and burgers", "Bread and rice", "Cheese and pasta"},
            0,  // Apples and almonds is correct (index 0)
            1300,
            {
                "Correct! Bees are the primary pollinators for apples and almonds.",
                "Not quite! Come back and try this flower again!",
                "Not quite! These foods are made from grains, but remember, bees pollinate flowers. Come back and try this flower again!",
                "Not quite! Remember, bees pollinate flowers. These foods are made from milk, grains and eggs. Come back and try this flower again!"
            }
        },

        // Question 8
        {
            "How do bees help animals like cows?",
            {"They clean cows", "They pollinate plants like alfalfa that cows eat", "They give cows honey", "They buzz around them"},
            1,  // They pollinate plants is correct (index 1)
            1400,
            {
                "Not quite! Hint– bees help with growing food for the cows. Come back and try this flower again!",
                "Correct! Bees pollinate many types of plants that animals eat, like alfalfa and clover that cows graze on.",
                "Not quite! Hint– bees help with growing food for the cows. Come back and try this flower again!",
                "Not quite! Hint– bees help with growing food for the cows. Come back and try this flower again!"
            }
        },

        // Question 9
        {
            "How much of our food comes from pollinators like bees?",
            {"One-fourth", "One-third", "One-half", "All of it"},
            1,  // One-third is correct (index 1)
            1600,
            {
                "Close, but not quite! It's actually a little more than that. Come back and try this flower again!",
                "Correct! About one-third of all the food we eat depends on pollinators like bees. That's why protecting them is so important!",
                "Not quite! It's actually a little less than that. Come back and try this flower again!",
                "Not quite! Come back and try this flower again!"
            }
        },

        // Question 10
        {
            "Why are bees called 'nature's tiny superheroes'?",
            {"They can fly really fast", "They wear capes", "They help organisms by pollinating", "They sting enemies"},
            2,  // They help organisms is correct (index 2)
            1700,
            {
                "Not quite! But it is fun to watch us zoom around, isn't it? Come back and try this flower again!",
                "Not quite! I wish I had a cape. Come back and try this flower again!",
                "Correct! Pollination helps plants grow more seeds and fruits. This means more food for animals and people!",
                "Not quite! It's true, we can fight enemies by stinging them, but we help nature in an even BIGGER way. Come back and try this flower again!"
            }
        },

        // Question 11
        {
            "What is one thing humans do that makes life hard for bees?",
            {"Planting more flowers", "Giving bees water", "Spraying harmful pesticides", "Building bee hotels"},
            2,  // Spraying pesticides is correct (index 2)
            1800,
            {
                "Not quite! This HELPS bees. But what do humans do that makes life hard? Come back and try this flower again!",
                "Not quite! This actually HELPS thirsty bees. But what do humans do that makes life hard? Come back and try this flower again!",
                "Correct. Most pesticides are dangerous for us. They can weaken, paralyze or even kill us.",
                "Not quite. Bee hotels are shelters humans can build out of tubes and sticks for bees to live in. This HELPS bees. Come back and try this flower again!"
            }
        },

        // Question 12
        {
            "What happens when bees lose their habitats?",
            {"They find new flowers quickly", "They lose their homes and food", "They build nests in the sky", "They become bigger"},
            1,  // They lose their homes is correct (index 1)
            1900,
            {
                "Not quite– when we lose our habitats, it's hard to find flowers. Come back and try this flower again!",
                "Yes– we lose our home and food. This is why habitat loss is such an important problem.",
                "Not quite– Come back and try this flower again!",
                "Not quite– Come back and try this flower again!"
            }
        },

        // Question 13
        {
            "How can climate change confuse bees?",
            {"It helps them fly faster", "It gives them new colors", "It causes strange weather that makes it harder to find food", "It makes flowers grow everywhere"},
            2,  // It makes it harder to find food is correct (index 2)
            2000,
            {
                "Not quite! Climate change makes life HARD for bees. Come back and try this flower again!",
                "Not quite! Climate change makes life HARD for bees. Come back and try this flower again!",
                "Correct– Climate change makes life HARD for bees, because it changes the environment in confusing ways.",
                "Not quite! Climate change makes life HARD for bees. Come back and try this flower again!"
            }
        },

        // Question 14
        {
            "Which insect attacks beehives and tries to take over?",
            {"Butterfly", "Murder Hornet", "Ladybug", "Bumblebee"},
            1,  // Murder Hornet is correct (index 1)
            2100,
            {
                "Not quite. Butterflies are actually pollinators, like us! They passed the vibe check. Come back and try this flower again!",
                "Correct. They are much bigger than us, so they can attack and destroy our nests. The only way to fight them is if we fight as a team! Let's do it right now.",
                "Not quite! Ladybugs actually eat aphids, which are critters who damage our plants. They're on our side fr fr. Come back and try this flower again!",
                "Not quite! Bumble bees are also pollinators, and they're like our cousins in the pollinator universe. No slander allowed. Come back and try this flower again!"
            }
        },

        // Question 15
        {
            "What tiny pest sticks to bees and makes them sick?",
            {"Termites", "Hive Beetles", "Varroa Mites", "Mosquitoes"},
            2,  // Varroa Mites is correct (index 2)
            2200,
            {
                "Not quite. Termites eat wood, and they don't really bother us. Come back and try this flower again!",
                "Not quite. Hive beetles are not tiny pests, they are biggies, and they break into our hives and steal our honey. They are NOT invited to the cookout. Come back and try this flower again!",
                "Correct! Varroa mites stick to us and make us sick. Top tier menace behavior, zero rizz, infinite red flags.",
                "Not quite. Mosquitoes don't really bother us. Come back and try this flower again!"
            }
        },

        // Question 16
        {
            "Which bug sneaks into hives and ruins the honey?",
            {"Hive Beetle", "Dragonfly", "Grasshopper", "Spider"},
            0,  // Hive Beetle is correct (index 0)
            2300,
            {
                "Correct! Hive beetles break into our hives and steal our honey. Major biggie energy.",
                "Not quite. Dragon flies are opps because they do eat bees, but they don't steal our honey. No biggie energy here. Come back and try this flower again!",
                "Not quite. Grasshoppers don't steal our honey. No biggie energy here. Come back and try this flower again!",
                "Not quite. Spiders are opps because they do catch and kill bees, but they don't steal our honey. No biggie energy here. Come back and try this flower again!"
            }
        },

        // Question 17
        {
            "How do bees defend themselves from big predators like murder hornets?",
            {"They fly away", "They hide in flowers", "They form a 'bee ball' to trap and overheat the enemy", "They sting each other"},
            2,  // They form a bee ball is correct (index 2)
            2400,
            {
                "Not quite! While bees do dodge the opps, they actually can to fight them, even huge predators like the murder hornet! Hint– there is strength in numbers. Come back and try this flower again!",
                "Not quite! While bees do dodge the opps, they actually can to fight them, even huge predators like the murder hornet! Hint– there is strength in numbers. Come back and try this flower again!",
                "Correct! Even though one bee is too small to fight a huge enemy, we can defeat them if we work as a team.",
                "Not quite! Hint– there is strength in numbers. Come back and try this flower again!"
            }
        },

        // Question 18
        {
            "Which of these is not a predator or danger to bees?",
            {"Hive Beetles", "Varroa Mites", "Murder Hornets", "Ladybugs"},
            3,  // Ladybugs is correct (index 3)
            2500,
            {
                "Not quite. Hive beetles break into our hives and steal our honey. Major opp behavior. Come back and try this flower again!",
                "Not quite. Varroa mites stick to us and make us sick. Major opp behavior. Come back and try this flower again!",
                "Not quite. Murder hornets attack and destroy our nests. Major opp behavior. Come back and try this flower again!",
                "Correct. Ladybugs eat aphids, which are critters who damage our plants. Green flags all the way."
            }
        }
    };

    // Initialize the pool of available questions
    available_questions = bee_questions;
}

TriviaQuestion TriviaManager::GetRandomQuestion() {
    // If no questions are left, reset the available questions
    if (available_questions.empty()) {
        available_questions = bee_questions;
    }

    // If still empty (which shouldn't happen), return an empty question
    if (available_questions.empty()) {
        return {};
    }

    // Create a distribution based on current available questions
    std::uniform_int_distribution<int> dist(0, available_questions.size() - 1);

    // Select a random question
    int index = dist(rng);
    TriviaQuestion selected_question = available_questions[index];

    // Remove the selected question from available questions
    available_questions.erase(available_questions.begin() + index);

    return selected_question;
}

bool TriviaManager::CheckAnswer(const TriviaQuestion& question, int selected_index) {
    return selected_index == question.correct_index;
}

std::string TriviaManager::GetExplanation(const TriviaQuestion& question, int selected_index) {
    // Make sure the index is valid
    if (selected_index >= 0 && selected_index < question.explanations.size()) {
        return question.explanations[selected_index];
    }
    return "No explanation available.";
}