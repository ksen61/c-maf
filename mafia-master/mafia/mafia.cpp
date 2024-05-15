#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

enum Role { MAFIA, CIVILIAN, DOCTOR, DETECTIVE };

class Player {
private:
    Role role;
    bool alive;
    bool protectedByDoctor;
    bool killedByMafia;
public:
    Player(Role r) : role(r), alive(true), protectedByDoctor(false), killedByMafia(false) {}

    Role getRole() const {
        return role;
    }

    bool isAlive() const {
        return alive;
    }

    bool isKilledByMafia() const {
        return killedByMafia;
    }

    void kill(bool byMafia = false) {
        if (!protectedByDoctor) {
            alive = false;
            if (byMafia) {
                killedByMafia = true;
            }
        }
        protectedByDoctor = false; // Сбрасываем защиту доктора после каждой ночи
    }

    void heal() {
        protectedByDoctor = true;
        killedByMafia = false; // Сбрасываем флаг "убит мафией" если доктор лечит
    }
};

class Game {
private:
    std::vector<Player> players;
    int mafiaCount;
    int civilianCount;
    bool detectedMafia; // флаг для отслеживания выявленной мафии детективом

public:
    Game(int playerCount) : detectedMafia(false) {
        mafiaCount = 1;
        civilianCount = playerCount - mafiaCount - 2; // Вычитаем 2 места для доктора и детектива

        for (int i = 0; i < playerCount; ++i) {
            if (i < mafiaCount)
                players.push_back(Player(MAFIA));
            else if (i < mafiaCount + 1)
                players.push_back(Player(DETECTIVE));
            else if (i < mafiaCount + 2)
                players.push_back(Player(DOCTOR));
            else
                players.push_back(Player(CIVILIAN));
        }

        std::shuffle(players.begin(), players.end(), std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count()));
    }

    void printRole(int playerIndex) {
        switch (players[playerIndex].getRole()) {
        case MAFIA:
            std::cout << "Вы мафия!\n";
            break;
        case CIVILIAN:
            std::cout << "Вы мирный житель!\n";
            break;
        case DOCTOR:
            std::cout << "Вы доктор!\n";
            break;
        case DETECTIVE:
            std::cout << "Вы детектив!\n";
            break;
        }
    }

    void mafiaTurn() {
        std::cout << "Мафия просыпается...\n";
        std::cout << "Выберите номер игрока для убийства: ";
        int target;
        std::cin >> target;
        target--; // Пользователь вводит номер с 1, а индексы начинаются с 0

        if (target < 0 || target >= players.size() || players[target].getRole() == MAFIA || !players[target].isAlive()) {
            std::cout << "Неверный выбор, повторите еще раз.\n";
            mafiaTurn();
            return;
        }

        std::cout << "Мафия выбирает игрока " << target + 1 << " для убийства.\n";
        players[target].kill(true);
        std::cout << "Мафия засыпает...\n";
    }

    void doctorTurn() {
        std::cout << "Доктор просыпается...\n";
        std::cout << "Выберите номер игрока для лечения: ";
        int target;
        std::cin >> target;
        target--; // Пользователь вводит номер с 1, а индексы начинаются с 0

        if (target < 0 || target >= players.size() || !players[target].isAlive()) {
            std::cout << "Неверный выбор, повторите еще раз.\n";
            doctorTurn();
            return;
        }

        std::cout << "Доктор выбирает игрока " << target + 1 << " для лечения.\n";
        players[target].heal();
        std::cout << "Доктор засыпает...\n";
    }

    void detectiveTurn() {
        std::cout << "Детектив просыпается...\n";
        std::cout << "Выберите номер игрока для расследования: ";
        int target;
        std::cin >> target;
        target--; // Пользователь вводит номер с 1, а индексы начинаются с 0

        if (target < 0 || target >= players.size() || !players[target].isAlive()) {
            std::cout << "Неверный выбор, повторите еще раз.\n";
            detectiveTurn();
            return;
        }

        std::cout << "Детектив выбирает игрока " << target + 1 << " для расследования.\n";
        if (players[target].getRole() == MAFIA) {
            std::cout << "Роль игрока " << target + 1 << ": Мафия\n";
            detectedMafia = true; // Ставим флаг, что мафия выявлена
        }
        else {
            std::cout << "Роль игрока " << target + 1 << ": Мирный житель\n";
        }
        std::cout << "Детектив засыпает...\n";
    }

    void vote() {
        std::cout << "Голосование...\n";
        std::vector<int> votes(players.size(), 0);

        for (int i = 0; i < players.size(); ++i) {
            if (players[i].isAlive() && !players[i].isKilledByMafia()) {
                std::cout << "Игрок " << i + 1 << ", выберите номер игрока для голосования: ";
                int voteTarget;
                std::cin >> voteTarget;
                voteTarget--; // Пользователь вводит номер с 1, а индексы начинаются с 0

                if (voteTarget < 0 || voteTarget >= players.size() || !players[voteTarget].isAlive() || players[voteTarget].isKilledByMafia()) {
                    std::cout << "Неверный выбор, голос не засчитан.\n";
                }
                else {
                    votes[voteTarget]++;
                }
            }
        }

        int maxVotes = 0;
        int playerToKill = -1;
        for (int i = 0; i < votes.size(); ++i) {
            if (votes[i] > maxVotes) {
                maxVotes = votes[i];
                playerToKill = i;
            }
        }

        if (playerToKill != -1) {
            std::cout << "Игрок " << playerToKill + 1 << " получает наибольшее количество голосов и будет убит.\n";
            if (players[playerToKill].getRole() == MAFIA && detectedMafia) {
                std::cout << "Игрок оказался мафией! Победили мирные жители!\n";
                exit(0); // Завершаем игру победой мирных жителей
            }
            players[playerToKill].kill();
        }
        else {
            std::cout << "Никто не был убит в этом раунде голосования.\n";
        }
    }

    void play() {
        bool mafiaWon = false;
        bool civiliansWon = false;

        while (!mafiaWon && !civiliansWon) {
            mafiaTurn();
            doctorTurn();
            detectiveTurn();

            // Проверяем, выжил ли кто-то после ночи
            int alivePlayers = 0;
            for (const auto& player : players) {
                if (player.isAlive()) {
                    alivePlayers++;
                }
            }

            if (alivePlayers <= 1) {
                break; // Игра завершена
            }

            vote();

            int mafiaAlive = 0;
            int otherRolesAlive = 0;

            for (const auto& player : players) {
                if (player.isAlive()) {
                    switch (player.getRole()) {
                    case MAFIA:
                        mafiaAlive++;
                        break;
                    case CIVILIAN:
                    case DOCTOR:
                    case DETECTIVE:
                        otherRolesAlive++;
                        break;
                    }
                }
            }

            if (mafiaAlive >= otherRolesAlive) {
                mafiaWon = true;
            }

            if (mafiaAlive == 0) {
                civiliansWon = true;
            }

            // Условие победы мафии при 3 оставшихся игроках
            if (alivePlayers == 3 && mafiaAlive > 0) {
                mafiaWon = true;
            }
        }

        if (mafiaWon) {
            std::cout << "Победила мафия!\n";
        }
        else {
            std::cout << "Победили мирные жители!\n";
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    int playerCount;

    while (true) {
        std::cout << "Введите количество игроков (от 5 до 7): ";
        std::cin >> playerCount;

        if (playerCount >= 5 && playerCount <= 7) {
            break;
        }

        std::cout << "Неправильное количество игроков! Попробуйте еще раз.\n";
    }

    Game game(playerCount);
    std::cout << "Ваша роль:\n";
    game.printRole(0);
    game.play();

    return 0;
}
