#include <stdio.h>
#include <fstream>
#include <tgbot/tgbot.h>

const std::string HELP_MESSAGE = "Капитан команды смотрит на ключ-карту и видит, какие из 25 разложенных на столе карточек относятся к своим агентам, какие к мирным жителям и какая карточка \
                                  обозначает в данной партии убийцу. После этого он дает своим игрокам подсказку в виде одного слова и одной цифры, которая обозначает количество карточек, \
                                  объединенных этим словом. \n Игроки совещаются и выбирают одно слово, которое, по их мнению, лучше всего подходит к подсказке. Если они отгадали верно, \
                                  капитан выкладывает на это слово карточку агента своего цвета, а игроки продолжают отгадывать слова. Если выбранная карточка оказывается мирным жителем, \
                                  то ход переходит к команде соперников. Если выбранная карточка оказывается агентом соперников, то ход переходит к ним и они получают это слово как отгаданное. \
                                  Если команда встречает убийцу, то она незамедлительно проигрывает.";

const std::string WORDS_FILE = "words.txt";
const int FIELD_SIZE = 5;

std::vector<std::string> words;

std::vector<int> k_random(int k, int l, int r) {
    if (k > r - l + 1) {
        throw std::invalid_argument("k bigger than range size");
    }
    std::vector<int> indices(k);
    std::vector<int> res(k);
    for (int i = 0; i < k; ++i) {
        indices[i] = rand() % (r - l + 1);
        int next = indices[i];
        for (int j = i - 1; j >= 0; --j) {
            if (indices[j] <= next) {
                ++next;
            }
        }
        res[i] = next; 
        --r;
    } 
    return res;
}

const int RED_WORDS = 8;
const int BLUE_WORDS = 9;
const int KILLER_WORDS = 1;
const std::string RED_CIRCLE = "\xF0\x9F\x94\xB4";
const std::string BLUE_CIRCLE = "\xF0\x9F\x94\xB5";
const std::string BLACK_CIRCLE = "\xE2\x9A\xAB";
const std::string WHITE_CIRCLE = "\xE2\x9A\xAA";

enum Word_type {
    red, blue, killer, none  
};

class Game {
    public:
        Game(int chat_id) {
            std::cout << "Game started" << std::endl;
            generate_field();
        }

        std::string field_to_string() {
            std::cout << "Converting field to string" << std::endl;
            std::string res;
            for (int i = 0; i < FIELD_SIZE; ++i) {
                for (int j = 0; j < FIELD_SIZE; ++j) {
                    switch (types[i][j]) {
                        case red : res += RED_CIRCLE; break;
                        case blue : res += BLUE_CIRCLE; break;
                        case killer : res += BLACK_CIRCLE; break;
                        case none : res += WHITE_CIRCLE; break;
                    }
                    res += " ";
                    res += field[i][j];
                    res += " ";
                }
                res += '\n';
            }
            std::cout << res << std::endl;
            return res;
        }

    private:
        std::vector<std::vector<std::string>> field;
        std::vector<std::vector<Word_type>> types;

        void generate_field() {
            std::cout << "Filling field with words" << std::endl;
            std::vector<int> words_num = k_random(FIELD_SIZE * FIELD_SIZE, 0, words.size() - 1);
            for (int i : words_num) {
                std::cout << i << ' ';
            }
            std::cout << std::endl;
            field.assign(FIELD_SIZE, std::vector<std::string>(FIELD_SIZE));
            for (int i = 0; i < FIELD_SIZE; ++i) {
                for (int j = 0; j < FIELD_SIZE; ++j) {
                    field[i][j] = words[words_num[i * FIELD_SIZE + j]];
                }
            }

            std::cout << "Filling field with types" << std::endl;
            std::vector<int> words_distribution = k_random(RED_WORDS + BLUE_WORDS + KILLER_WORDS, 0, FIELD_SIZE * FIELD_SIZE - 1);
            types.assign(FIELD_SIZE, std::vector<Word_type>(FIELD_SIZE, none));
            for (int i = 0; i < RED_WORDS; ++i) {
                int num = words_distribution[i];
                types[num / FIELD_SIZE][num % FIELD_SIZE] = red;
            }
            
            for (int i = 8; i < 8 + BLUE_WORDS; ++i) {
                int num = words_distribution[i];
                types[num / FIELD_SIZE][num % FIELD_SIZE] = blue;
            }
            
            int num = words_distribution.back();
            types[num / FIELD_SIZE][num % FIELD_SIZE] = killer;
        }

        /* std::string field_to_string() {
            std::string res;
            for (int i = 0; i < FIELD_SIZE; ++i) {
                for (int j = 0; j < FIELD_SIZE; ++j) {
                    switch (types[i][j]) {
                        case red : res += RED_CIRCLE; break;
                        case blue : res += BLUE_CIRCLE; break;
                        case killer : res += BLACK_CIRCLE; break;
                        case none : res += WHITE_CIRCLE; break;
                    }
                    res += " ";
                    res += words[i][j];
                    res += " ";
                }
                res += '\n';
            }
            return res;
        } */
};

void read_words() {
    std::ifstream words_input(WORDS_FILE);
    std::string next_word;
    while (words_input >> next_word) {
        words.push_back(next_word);
    }
    words_input.close();
}

int main() {
    std::ifstream token_input("token.txt");
    std::string token;
    token_input >> token;
    read_words();
    TgBot::Bot bot(token);
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });
    bot.getEvents().onCommand("help", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, HELP_MESSAGE);
    });
    bot.getEvents().onCommand("start_game", [&bot](TgBot::Message::Ptr message) {
        Game new_game(228);
        bot.getApi().sendMessage(message->chat->id, new_game.field_to_string());
    });
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}
